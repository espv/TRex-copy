//
// Created by espen on 20.11.18.
//

#ifndef TREXSERVER_TRACE_FRAMEWORK_H
#define TREXSERVER_TRACE_FRAMEWORK_H

#include <chrono>
#include <sched.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

using namespace std;

bool doTrace = false;
pthread_mutex_t* traceMutex = new pthread_mutex_t;
double previous_time;
long long first_time = 0;

class TraceEvent {
public:
    int locationId;
    int cpuId;
    int threadId;
    long long timestamp;
};

TraceEvent events[1000000];
int tracedEvents = 0;

bool writeTracesToFile = true;
void traceEvent(int traceId, bool reset)
{
    if (!doTrace)
        return;
    int pid = syscall(SYS_gettid);
    pthread_mutex_lock(traceMutex);
    auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
    if (writeTracesToFile) {
        events[tracedEvents].locationId = traceId;
        events[tracedEvents].cpuId = sched_getcpu();
        events[tracedEvents].threadId = pid;
        events[tracedEvents].timestamp = current_time;
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

#endif //TREXSERVER_TRACE_FRAMEWORK_H
