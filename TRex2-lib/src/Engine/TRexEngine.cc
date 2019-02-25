//
// This file is part of T-Rex, a Complex Event Processing Middleware.
// See http://home.dei.polimi.it/margara
//
// Authors: Alessandro Margara, Daniele Rogora
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "TRexEngine.h"
#include "../Common/trace-framework.hpp"
#include <sys/time.h>
#include <sys/syscall.h>
#include <chrono>

using namespace std;


void* processor(void* parShared) {
  Shared* s = (Shared*)parShared;
  bool first = true;
  traceEvent(57);
  while (true) {
    if (first) {
      pthread_mutex_lock(s->processMutex);
      first = false;
    }

    //traceEvent(58);
    pthread_cond_wait(s->processCond, s->processMutex);
    // At this point, s->processMutex is locked by this thread, and s->lowerBound and s->upperBound will not be changed by TRexEngine::processRulePkt
    int lowerBound = s->lowerBound;
    int upperBound = s->upperBound;
    traceEvent(6, 1);
    // End processing
    if (s->finish) {
      pthread_mutex_unlock(s->processMutex);
      break;
    }
    pthread_mutex_unlock(s->processMutex);
    MatchingHandler* mh = s->mh;
    set<PubPkt*> generatedPkts;
    for (int i = lowerBound; i < upperBound; i++) {
      traceEvent(7);
      auto negsIt = mh->matchingNegations.find(i);
      if (negsIt != mh->matchingNegations.end()) {
        for (auto index : negsIt->second) {
          s->stacksRule->find(i)->second->addToNegationStack(s->pkt, index);
        }
      }
      auto aggsIt = mh->matchingAggregates.find(i);
      if (aggsIt != mh->matchingAggregates.end()) {
        for (auto index : aggsIt->second) {
          s->stacksRule->find(i)->second->addToAggregateStack(s->pkt, index);
        }
      }

      auto statesIt = mh->matchingStates.find(i);
      if (statesIt != mh->matchingStates.end()) {
        // Last state (if found) is processed last
        bool lastState = false;
        for (auto index : statesIt->second) {
          if (index != 0) {
            s->stacksRule->find(i)->second->addToStack(s->pkt, index);
          } else {
            lastState = true;
          }
        }
        if (lastState) {
          s->stacksRule->find(i)->second->startComputation(s->pkt, s->result);
        }
      }
    }
    traceEvent(59);
    pthread_mutex_lock(s->processMutex);
    pthread_mutex_lock(s->resultMutex);
    *(s->stillProcessing) = *(s->stillProcessing) - 1;
    if (*(s->stillProcessing) == 0)
      pthread_cond_signal(s->resultCond);
    pthread_mutex_unlock(s->resultMutex);

  }
  pthread_exit(NULL);
}

TRexEngine::TRexEngine(int parNumProc) {
  numProc = parNumProc;
  threads = new pthread_t[numProc];
  stacksRules = new StacksRules;
  shared = new Shared[numProc];
  recursionNeeded = false;
}

TRexEngine::~TRexEngine() {
  for (int i = 0; i < numProc; i++) {
    shared[i].finish = true;
    pthread_mutex_lock(shared[i].processMutex);
    pthread_cond_signal(shared[i].processCond);
    pthread_mutex_unlock(shared[i].processMutex);
    pthread_join(threads[i], NULL);
    pthread_mutex_destroy(shared[i].processMutex);
    pthread_cond_destroy(shared[i].processCond);
    delete shared[i].processMutex;
    delete shared[i].processCond;
  }
  pthread_mutex_destroy(shared[0].resultMutex);
  pthread_cond_destroy(shared[0].resultCond);
  delete shared[0].resultMutex;
  delete shared[0].resultCond;
  delete shared[0].stillProcessing;
  delete[] shared;
  delete[] threads;
  for (auto it : *stacksRules) {
    StacksRule* stackRule = it.second;
    delete stackRule;
  }
  delete stacksRules;
}

void TRexEngine::finalize() {
  // Creates shared memory and initializes threads
  int size = stacksRules->size() / numProc + 1;
  int* stillProcessing = new int;
  pthread_mutex_t *resultMutex = new pthread_mutex_t;
  pthread_cond_t *resultCond = new pthread_cond_t;
  (*stillProcessing) = numProc;
  pthread_mutex_init(resultMutex, NULL);
  pthread_cond_init(resultCond, NULL);

  for (int i = 0; i < numProc; i++) {
    shared[i].lowerBound = size * i;
    shared[i].upperBound = size * i;
    shared[i].finish = false;
    shared[i].stacksRule = stacksRules;
    shared[i].stillProcessing = stillProcessing;
    shared[i].processMutex = new pthread_mutex_t;
    shared[i].processCond = new pthread_cond_t;
    shared[i].resultMutex = resultMutex;
    shared[i].resultCond = resultCond;
    pthread_mutex_init(shared[i].processMutex, NULL);
    pthread_cond_init(shared[i].processCond, NULL);
    pthread_create(&threads[i], NULL, processor, (void *) &shared[i]);
  }
  usleep(1000);
}


void TRexEngine::setRecursionNeeded(RulePkt* pkt) {
  // If this is already true there's nothing to do here
  if (recursionNeeded == true)
    return;

  for (int i = 0; i < pkt->getPredicatesNum(); i++)
    inputEvents.insert(pkt->getPredicate(i).eventType);
  outputEvents.insert(pkt->getCompositeEventTemplate()->getEventType());

  for (auto it : inputEvents) {
    if (outputEvents.find(it) != outputEvents.end()) {
      recursionNeeded = true;
      return;
    }
  }
}

void TRexEngine::processRulePkt(RulePkt* pkt) {
  setRecursionNeeded(pkt);
  StacksRule* stacksRule = new StacksRule(pkt);
  stacksRules->insert(make_pair(stacksRule->getRuleId(), stacksRule));
  int size = stacksRules->size() / numProc;
  for (int i = 0; i < numProc; i++) {
    pthread_mutex_lock(shared[i].processMutex);
    // The processor function will only access shared[i].lowerBound and shared[i].upperBound after locking shared[i].processMutex
    shared[i].lowerBound = size * i;
    shared[i].upperBound = size * (i + 1);
    pthread_mutex_unlock(shared[i].processMutex);
  }
  indexingTable.installRulePkt(pkt);
  delete pkt;
}

void TRexEngine::processPubPkt(PubPkt* pkt, bool recursion) {
  if (recursion == false)
    recursionDepth = 0;
  else
    recursionDepth++;
  // Data structures used to compute mean processing time
  timeval tValStart, tValEnd;
  gettimeofday(&tValStart, NULL);

  // Obtains the set of all interested sequences and states
  MatchingHandler* mh = new MatchingHandler;
  indexingTable.processMessage(pkt, *mh);
  set<PubPkt*> result;

  traceEvent(110);
  // Installs information in shared memory
  for (int i = 0; i < numProc; i++) {
    traceEvent(5);
    pthread_mutex_lock(shared[i].processMutex);
    shared[i].mh = mh;
#if MP_MODE == MP_COPY
    shared[i].pkt = pkt->copy();
#elif MP_MODE == MP_LOCK
    shared[i].pkt = pkt;
#endif
    pthread_cond_signal(shared[i].processCond);
    pthread_mutex_unlock(shared[i].processMutex);
  }

  // Waits until all processes finish
  pthread_mutex_lock(shared[0].resultMutex);
  // If not all thread have finished, wait until last one
  //traceEvent(8);
  if (*(shared[0].stillProcessing) != 0)
    pthread_cond_wait(shared[0].resultCond, shared[0].resultMutex);
  pthread_mutex_unlock(shared[0].resultMutex);
  *(shared[0].stillProcessing) = numProc;
  for (int i = 0; i < numProc; i++)  // Part of tracing only to connect trace event 8 with 111 for all processes
    traceEvent(111, 1);

  // Collects results
  for (int i = 0; i < numProc; i++) {
    for (auto resPkt : shared[i].result) {
      result.insert(resPkt);
    }
    shared[i].result.clear();
#if MP_MODE == MP_COPY
    if (shared[i].pkt->decRefCount())
      delete shared[i].pkt;
#endif
  }
  // Deletes used packet
  if (pkt->decRefCount())
    delete pkt;
  delete mh;

  gettimeofday(&tValEnd, NULL);
  double duration = (tValEnd.tv_sec - tValStart.tv_sec) * 1000000 +
                    tValEnd.tv_usec - tValStart.tv_usec;

  traceEvent(9);
  // Notifies results to listeners
  for (auto listener : resultListeners) {
    traceEvent(10);
    listener->handleResult(result, duration);
  }
  traceEvent(11);

  for (auto pkt : result) {
    if (recursionNeeded && recursionDepth < MAX_RECURSION_DEPTH)
      processPubPkt(pkt->copy(), true);
    if (pkt->decRefCount()) {
      //delete pkt;  // Added by Espen for experiments. We do not want to delete the packets since we'll reuse them.
    }
  }
  traceEvent(12);
}

void TRexEngine::processPubPkt(PubPkt* pkt) {
  return processPubPkt(pkt, false);
}
