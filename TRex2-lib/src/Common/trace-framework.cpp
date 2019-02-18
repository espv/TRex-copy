//
// Created by espen on 13.12.18.
//


#include <chrono>
#include <sched.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdint.h>
#include "trace-framework.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

// For creating a unique trace file name
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace boost::uuids;

using namespace std;

bool doTrace = false;
pthread_mutex_t* traceMutex = new pthread_mutex_t;
double previous_time;
long long first_time = 0;

int tracedEvents = 0;

#define MAX_NUMBER_EVENTS 100000
TraceEvent events[MAX_NUMBER_EVENTS];

bool writeTraceToFile = true;
void traceEvent(int traceId, int eventType, bool reset)
{
    if (!doTrace || (traceId != 1 && traceId != 100 && traceId != 50 && traceId != 51 && traceId != 155 && traceId != 5 && traceId != 6 && traceId != 7 && traceId != 12 && traceId != 57 && traceId != 58 && traceId != 59 && traceId != 110 && traceId != 111 && traceId != 230))
        return;
    int pid = syscall(SYS_gettid);
    pthread_mutex_lock(traceMutex);
    auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
    if (writeTraceToFile) {
        if (tracedEvents >= MAX_NUMBER_EVENTS -1)
            writeBufferToFile();
        events[tracedEvents].locationId = traceId;
        events[tracedEvents].cpuId = sched_getcpu();
        events[tracedEvents].threadId = pid;
        events[tracedEvents].timestamp = current_time;
        events[tracedEvents].type = eventType;
        ++tracedEvents;
    } else {
        if (first_time == 0)
            first_time = current_time;
        // Resetting doesn't work currently since multiple cores and threads might call this same function
        // The exception is when tracing in low event rates
        if (reset)
            previous_time = current_time;
        cout << traceId << "-" << sched_getcpu() << "-" << pid << "-" << current_time - first_time << std::endl;
        previous_time = current_time;
    }
    pthread_mutex_unlock(traceMutex);
};

int trace_index = 0;
void writeBufferToFile()
{
    ofstream myfile;
    std::ostringstream oss;
    random_generator gen;
    uuid id = gen();
    oss << "../analysis/traces/" << std::time(0) << "-" << id << "-" << trace_index++ << ".trace";
    std::string fn = oss.str();
    myfile.open (fn);
    for (int i = 0; i < tracedEvents; ++i)
    {
        TraceEvent *event = &events[i];
        myfile << event->locationId << "\t" << event->type << "\t" << event->cpuId << "\t" << event->threadId << "\t" << event->timestamp << "\n";
    }
    myfile.close();
    exit(0);
}
