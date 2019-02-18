//
// Created by espen on 20.11.18.
//

#ifndef TREXSERVER_TRACE_FRAMEWORK_H
#define TREXSERVER_TRACE_FRAMEWORK_H

class TraceEvent {
public:
    int locationId;
    int cpuId;
    int threadId;
    int type;
    long long timestamp;
    long long rdtsc;
};

void traceEvent(int traceId, int eventType=0, bool reset=false);

void writeBufferToFile();

#endif //TREXSERVER_TRACE_FRAMEWORK_H
