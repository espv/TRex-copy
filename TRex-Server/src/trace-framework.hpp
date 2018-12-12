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
#include <stdint.h>

//  Windows
#ifdef _WIN32

#include <intrin.h>
uint64_t rdtsc(){
    return __rdtsc();
}

//  Linux/GCC
#else

uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif

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
    long long rdtsc;
};

TraceEvent events[1000000];
int tracedEvents = 0;

bool writeTraceToFile = true;
void traceEvent(int traceId, bool reset)
{
    if (!doTrace || (traceId != 1 && traceId != 100))
        return;
    int pid = syscall(SYS_gettid);
    pthread_mutex_lock(traceMutex);
    auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
    if (writeTraceToFile) {
        events[tracedEvents].locationId = traceId;
        events[tracedEvents].cpuId = sched_getcpu();
        events[tracedEvents].threadId = pid;
        events[tracedEvents].timestamp = current_time;
        events[tracedEvents].rdtsc = rdtsc();
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
