/*
 * Copyright (C) 2011 Francesco Feltrinelli <first_name DOT last_name AT gmail DOT com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RequestHandler.hpp"
#include "../../../TRex2-lib/src/Common/trace-framework.hpp"
#include "../../../TRex2-lib/src/Packets/PubPkt.h"
#include <sys/syscall.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include <cstring>


using concept::connection::RequestHandler;
using namespace concept::packet;
using namespace concept::util;
using namespace std;

#ifdef HAVE_GTREX
	RequestHandler::RequestHandler(TRexEngine &tRexEngine, GPUEngine &gtRexEngine, ConnectionProxy& connection, SubscriptionTable &subTable, bool useGPUPar):
	tRexEngine(tRexEngine),
	gtRexEngine(gtRexEngine),
	subTable(subTable),
	connection(connection),
	resultListener(*this),
	firstSubscriptionDone(false),
	useGPU(useGPUPar)
	{}
#else
	RequestHandler::RequestHandler(TRexEngine &tRexEngine, ConnectionProxy& connection, SubscriptionTable &subTable, bool useGPUPar):
	tRexEngine(tRexEngine),
	subTable(subTable),
	connection(connection),
	resultListener(*this),
	firstSubscriptionDone(false),
	useGPU(useGPUPar)
	{}
#endif

/*long long cnt = 0;
PubPkt allPackets[100];
RequestHandler *p;
auto prev_time = std::chrono::system_clock::now().time_since_epoch().count();
void HandlePubPacket(const boost::system::error_code&)
{
	std::time_t now = std::time(0);
	boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
	PubPkt *pkt = &allPackets[gen()%100];
	traceEvent(1, true);
	if (++cnt % 2000 == 0) {
		auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
		std::cout << cnt << " - " << current_time-prev_time << std::endl;
		prev_time = current_time;
	}
	//LOG(info) << "Publication from " << p.connection.remoteToString() << ":" << endl
	//          << "  " << toString(pkt);
	//continue;
	//PubPkt *newPkt = pkt->copy();
	p->subTable.processPublication(pkt);
	// Let TRex process (and *delete*) the packet
	p->tRexEngine.processPubPkt(pkt);
	//pkt = newPkt;
	traceEvent(100);
#ifdef HAVE_GTREX
	if (useGPU) {
          parent.gtRexEngine.processPubPkt(pkt);
        }
#endif
}*/

RequestHandler::~RequestHandler(){
	/*boost::asio::io_service io;

	boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));
    for (int i = 0; i < 100; i++) {
        PubPkt *pkt = &allPackets[i];
        pkt->eventType = i % 20 + 2;
        pkt->attributesNum = 2;
        pkt->attributes = new Attribute[2];
		//strncpy(pkt->attributes[0].name, "value", sizeof(pkt->attributes[0].name) - 1);

        pkt->attributes[0].name[0] = 'v';
        pkt->attributes[0].name[1] = 'a';
        pkt->attributes[0].name[2] = 'l';
        pkt->attributes[0].name[3] = 'u';
        pkt->attributes[0].name[4] = 'e';
        pkt->attributes[0].name[5] = '\0';
        pkt->attributes[0].intVal = 98;

		//strlcpy(pkt->attributes[1].name, "area", sizeof(pkt->attributes[1].name) - 1);
		//strlcpy(pkt->attributes[1].stringVal, "office", sizeof(pkt->attributes[1].stringVal) - 1);

		pkt->attributes[1].name[0] = 'a';
		pkt->attributes[1].name[1] = 'r';
		pkt->attributes[1].name[2] = 'e';
		pkt->attributes[1].name[3] = 'a';
		pkt->attributes[1].name[4] = '\0';
		pkt->attributes[1].stringVal[0] = 'o';
		pkt->attributes[1].stringVal[1] = 'f';
		pkt->attributes[1].stringVal[2] = 'f';
		pkt->attributes[1].stringVal[3] = 'i';
		pkt->attributes[1].stringVal[4] = 'c';
		pkt->attributes[1].stringVal[5] = 'e';
		pkt->attributes[1].stringVal[6] = '\0';
    }

	t.async_wait(&HandlePubPacket);*/
	if (firstSubscriptionDone) {
		if (!useGPU) tRexEngine.removeResultListener(&resultListener);
#ifdef HAVE_GTREX
		if (useGPU) gtRexEngine.removeResultListener(&resultListener);
#endif
		subTable.removeListener(this);
		LOG(info) << "Removed subscriptions for " << connection.remoteToString() << endl;
	}
}

void RequestHandler::handleRequest(std::vector<PktPtr> & pkts){
	for (std::vector<PktPtr>::iterator it= pkts.begin(); it != pkts.end(); it++){
		boost::apply_visitor(PktHandleVisitor(*this, useGPU), *it);
	}
}

void RequestHandler::PktHandleVisitor::operator()(RulePkt * pkt) const{
	//p = &parent;
	LOG(info) << "Rule from " << parent.connection.remoteToString() << ":" << endl
			<< "  " << toString(pkt);
	// Let TRex process (and *delete*) the packet
	if (!useGPU) {
	  parent.tRexEngine.processRulePkt(pkt);
	}
#ifdef HAVE_GTREX
	if (useGPU) {
	  parent.gtRexEngine.processRulePkt(pkt);
	}
#endif
}

long long pktsProcessed = 0;
auto prev_time_processed = std::chrono::system_clock::now().time_since_epoch().count();
void RequestHandler::PktHandleVisitor::operator()(PubPkt * pkt) const{
    traceEvent(1, true);
    if (++pktsProcessed % 2000 == 0) {
        auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << pktsProcessed << " - " << current_time-prev_time_processed << std::endl;
      prev_time_processed = current_time;
    }
    //LOG(info) << "Publication from " << parent.connection.remoteToString() << ":" << endl
    //          << "  " << toString(pkt);
    //continue;
    //PubPkt *newPkt = pkt->copy();
    parent.subTable.processPublication(pkt);
    // Let TRex process (and *delete*) the packet
    if (!useGPU) parent.tRexEngine.processPubPkt(pkt);
    //pkt = newPkt;
    traceEvent(100);
#ifdef HAVE_GTREX
    if (useGPU) {
          parent.gtRexEngine.processPubPkt(pkt);
        }
#endif
}

void RequestHandler::PktHandleVisitor::operator()(SubPkt * pkt) const{
	LOG(info) << "Subscription from " << parent.connection.remoteToString() << ":" << endl
			<< "  " << toString(pkt);

	// Check if subscription was already done
	bool alreadySubscribed= false;
	for (vector<SubPkt>::iterator it= parent.subscriptions.begin(); it!= parent.subscriptions.end(); it++){
		if (equals(&(*it), pkt)) {
			alreadySubscribed= true;
			break;
		}
	}
	// If subscription is new add a *copy* of it
	if (!alreadySubscribed) parent.subscriptions.push_back(*pkt);

	delete pkt;

	if (!parent.firstSubscriptionDone){
		if (!useGPU) parent.tRexEngine.addResultListener(&parent.resultListener);
#ifdef HAVE_GTREX
		if (useGPU) {
		  parent.gtRexEngine.addResultListener(&parent.resultListener);
		}
#endif
		parent.subTable.addListener(&parent);
		parent.firstSubscriptionDone= true;
	}
}

void RequestHandler::PktHandleVisitor::operator()(UnSubPkt * pUnsub) const{
	stringstream log;
	log << "Requested Unsubscription from " << parent.connection.remoteToString() << " for:" << endl
			<< "  " << toString(pUnsub->getSubPkt()) << endl;

	bool wasSubscribed= false;
	for (vector<SubPkt>::iterator subIt= parent.subscriptions.begin(); subIt!= parent.subscriptions.end(); subIt++){
		SubPkt* pSub= &(*subIt);
		if (equals(pSub, pUnsub->getSubPkt())){
			wasSubscribed= true;
			parent.subscriptions.erase(subIt);
			break;
		}
	}

	if (wasSubscribed) log << "  .. subscription removed.";
	else log << "  .. was not subscribed.";
	LOG(info) << log.str();

	delete pUnsub;
}

void RequestHandler::PktHandleVisitor::operator()(PingPkt * pkt) const{
	LOG(debug) << "Ping from " << parent.connection.remoteToString();
	delete pkt;
}

void RequestHandler::ResultListenerImpl::handleResult(set<PubPkt *> &genPkts, double procTime){
	stringstream log;
	for (set<PubPkt*>::iterator pubIt= genPkts.begin(); pubIt != genPkts.end(); pubIt++){
		PubPkt* pPub= *pubIt;
		for (vector<SubPkt>::iterator subIt= parent.subscriptions.begin(); subIt!= parent.subscriptions.end(); subIt++){
			SubPkt* pSub= &(*subIt);
			if (matches(pSub, pPub)){
				log << endl << "  - " << toString(pPub);
				CharVectorPtr bytes = parent.marshaller.marshal(PktPtr(pPub));
				parent.connection.asyncWrite(bytes);
			}
		}
		// no need to delete the PubPkts: they will be deleted by TRex
	}

	if (log.gcount() > 0){
		LOG(debug) << "Published to " << parent.connection.remoteToString() << ":" << log.str();
	}
}

void RequestHandler::processPubPkt(PubPkt *pkt) {
	// Match the publication against installed subscriptions
	for (vector<SubPkt>::iterator subIt=subscriptions.begin(); subIt!=subscriptions.end(); subIt++){
		SubPkt* pSub= &(*subIt);
		if (matches(pSub, pkt)){
			CharVectorPtr bytes = marshaller.marshal(PktPtr(pkt));
			connection.asyncWrite(bytes);
		}
	}
}
