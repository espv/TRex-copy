//
// Created by espen on 20.11.18.
//

#ifndef TREXSERVER_TRACE_FRAMEWORK_H
#define TREXSERVER_TRACE_FRAMEWORK_H

#include <chrono>

void traceEvent(int traceId, int pid, bool reset);

bool doTrace = false;
pthread_mutex_t* traceMutex = new pthread_mutex_t;
double previous_time;
void traceEvent(int traceId, int pid, bool reset)
{
    if (!doTrace)
        return;
    pthread_mutex_lock(traceMutex);
    auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
    if (reset)
        previous_time = current_time;
    cout << traceId << "-" << pid << "-" << current_time-previous_time << std::endl;
    previous_time = current_time;
    pthread_mutex_unlock(traceMutex);
};

#endif //TREXSERVER_TRACE_FRAMEWORK_H
