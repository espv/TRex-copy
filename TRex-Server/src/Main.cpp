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

#include "external.hpp"
#include "server.hpp"
#include "test.hpp"
#include "util.hpp"
#include "../../TRex2-lib/src/Common/trace-framework.hpp"
#include "../../TRex2-lib/src/Packets/PubPkt.h"
#include "../../TRex2-lib/src/Engine/TRexEngine.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <sys/syscall.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include <boost/thread.hpp>
#include <cstring>

using concept::server::SOEPServer;
using namespace concept::test;
using concept::util::Logging;
using namespace std;

void my_handler(int s){
	writeBufferToFile();
	exit(1);
}


int number_threads = boost::thread::hardware_concurrency();
void runServer(bool useGPU){
	// Create server with default port and #threads = #CPUs
#ifdef USE_SINGLE_CORE
    std::cout << "Using 2 thread" << std::endl;
	SOEPServer server(SOEPServer::DEFAULT_PORT, 5, false, useGPU);
# else
	std::cout << "Using " << number_threads << " threads" << std::endl;
	SOEPServer server(SOEPServer::DEFAULT_PORT, number_threads, false, useGPU);
#endif

	server.run();
}

#define PACKET_CAPACITY 10
int cur_pkt_index = 0;
int number_dropped_packets = 0;
int number_placed_packets = 0;
std::vector<PubPkt*> packetQueue;

long long cnt = 0;
std::vector<PubPkt*> allPackets;
auto prev_time = std::chrono::system_clock::now().time_since_epoch().count();
TRexEngine *this_engine;
boost::posix_time::microsec interval(10000);
boost::asio::io_service io;
boost::asio::deadline_timer t(io, interval);

boost::interprocess::interprocess_semaphore packet_thread_semaphore(0);
std::time_t now = std::time(0);
boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};

void HandlePubPacket(const boost::system::error_code&)
{
    if (packetQueue.size() < PACKET_CAPACITY) {
        ++cur_pkt_index;
        ++number_placed_packets;
        if (number_placed_packets % 100000 == 0)
            std::cout << "Inserted packet #" << number_placed_packets << " into queue" << std::endl;
        packetQueue.push_back(new PubPkt(*allPackets.at(gen()%100)));
        packet_thread_semaphore.post();
    } else {
        ++number_dropped_packets;
        if (number_dropped_packets % 10000 == 0)
            std::cout << "Dropped " << number_dropped_packets << " packets" << std::endl;
    }

    t.expires_at(t.expires_at() + interval);
    t.async_wait(&HandlePubPacket);

    /*
    traceEvent(1, true);
    if (++cnt % 2000 == 0) {
        auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << cnt << " - " << current_time-prev_time << std::endl;
        prev_time = current_time;
    }

    this_engine->processPubPkt(pkt);
    traceEvent(100, true);*/
}

TRexEngine *engine;
RuleR1 testRule;

void
PublishPackets()
{
    while (true) {
        packet_thread_semaphore.wait();
        //if (packetQueue.size() > 0) {
        //std::cout << "Handling a published packet with size " << packetQueue.size() << std::endl;
        PubPkt *pkt = packetQueue.front();
        traceEvent(1, true);
        if (++cnt % 100000 == 0) {
            auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
            std::cout << cnt << " - " << current_time - prev_time << std::endl;
            prev_time = current_time;
        }

        engine->processPubPkt(pkt);
        traceEvent(100, true);
        //std::cout << "Freeing up packet in queue with size " << packetQueue.size() << std::endl;
        packetQueue.erase(packetQueue.begin());
        //std::cout << "Finished processing packet" << std::endl;
        //}
    }
}

void sendTestPackets() {
    vector<PubPkt*> pubPkts= testRule.buildPublication();
    for (vector<PubPkt*>::iterator it= pubPkts.begin(); it != pubPkts.end(); it++){
        engine->processPubPkt(*it);
    }
}

void testEngine(){
    engine = new TRexEngine(number_threads);
    engine->finalize();

    for (int i = 0; i < 1000; i++)
	    engine->processRulePkt(testRule.buildRule());

	ResultListener* listener= new TestResultListener(testRule.buildSubscription());
	engine->addResultListener(listener);

	allPackets = testRule.buildPublication();

	/*
    for (int i = 0; i < 100; i++) {
        auto attributes = new Attribute[2];
        //strncpy(attributes[0].name, "value", sizeof(attributes[0].name) - 1);

        attributes[0].name[0] = 'v';
        attributes[0].name[1] = 'a';
        attributes[0].name[2] = 'l';
        attributes[0].name[3] = 'u';
        attributes[0].name[4] = 'e';
        attributes[0].name[5] = '\0';
        attributes[0].intVal = 98;

        //strlcpy(attributes[1].name, "area", sizeof(attributes[1].name) - 1);
        //strlcpy(attributes[1].stringVal, "office", sizeof(attributes[1].stringVal) - 1);

        attributes[1].name[0] = 'a';
        attributes[1].name[1] = 'r';
        attributes[1].name[2] = 'e';
        attributes[1].name[3] = 'a';
        attributes[1].name[4] = '\0';
        attributes[1].stringVal[0] = 'o';
        attributes[1].stringVal[1] = 'f';
        attributes[1].stringVal[2] = 'f';
        attributes[1].stringVal[3] = 'i';
        attributes[1].stringVal[4] = 'c';
        attributes[1].stringVal[5] = 'e';
        attributes[1].stringVal[6] = '\0';
        PubPkt *pkt = new PubPkt(i % 20 + 2, attributes, 2);
        allPackets.push_back(pkt);
    }*/
    for (int i = 0; i < number_threads; ++i) {
        boost::thread th{PublishPackets};
    }
    t.async_wait(&HandlePubPacket);
    io.run();
    //while (true);

	/* Expected output: complex event should be created by T-Rex and published
	 * to the TestResultListener, which should print it to screen.
	 */
}

extern bool doTrace;
int main(int argc, char* argv[]){
	bool test = false;
	for (int i = 1; i < argc; i++) {
	    if (!strcmp(argv[i], "-trace"))
	        doTrace = true;
	    else if (!strcmp(argv[i], "-number-threads")) {
	        ++i;
            number_threads = std::atoi(argv[i]);
        } else if (!strcmp(argv[i], "-test"))
	    	test = true;
	}

    boost::log::core::get()->set_logging_enabled(false);
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	Logging::init();
#ifdef HAVE_GTREX
	if (argc==2 && strcmp(argv[1], "-gpu")==0) {
	  cout << "Using GPU engine - GTREX" << endl;
	  runServer(true);
	}
	else {
	  cout << "Using CPU engine - TREX" << endl;
	  runServer(false);
	}
#else
	if (test)
		testEngine();
	else
		runServer(false);
#endif
}
