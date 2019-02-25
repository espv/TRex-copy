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
#include "Test/RuleR0.hpp"
#include <boost/thread.hpp>
#include <cstring>
#include <ctime>

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


int number_dropped_packets = 0;
int number_placed_packets = 0;
#define PACKET_CAPACITY 10
std::vector<PubPkt*> packetQueue;

long long pktsPublished = 0;
std::vector<PubPkt*> allPackets;
auto prev_time_published = std::chrono::system_clock::now().time_since_epoch().count();
TRexEngine *this_engine;
boost::posix_time::microsec interval(20000);
boost::asio::io_service io;
boost::asio::deadline_timer t(io, interval);

std::time_t now = std::time(0);
boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
auto packetQueueMutex = new pthread_mutex_t;

void HandlePubPacket(const boost::system::error_code&)
{
	//std::cout << "HandlePubPacket" << std::endl;
	if (packetQueue.size() < PACKET_CAPACITY) {
		if (++number_placed_packets % 10000 == 0)
			std::cout << "Inserted packet #" << number_placed_packets << " into queue" << std::endl;
		pthread_mutex_lock(packetQueueMutex);
		packetQueue.push_back(new PubPkt(*allPackets.at(gen()%allPackets.size())));
		pthread_mutex_unlock(packetQueueMutex);
		std::cout << "Inserted packet #" << number_placed_packets << " into queue" << std::endl;
	} else {
		++number_dropped_packets;
		if (number_dropped_packets % 10000 == 0)
			std::cout << "Dropped " << number_dropped_packets << " packets" << std::endl;
	}

	t.expires_at(t.expires_at() + interval);
	t.async_wait(&HandlePubPacket);
}

void PublishPackets()
{
	while (true) {
		//std::cout << "PublishPackets, size of packetQueue: " << packetQueue.size() << std::endl;
		if (packetQueue.size() > 0) {
			//std::cout << "PublishPackets, size of packetQueue: " << packetQueue.size() << std::endl;
			PubPkt *pkt = packetQueue.front();
			traceEvent(1, 0, true);
			if (++pktsPublished % 2000 == 0) {
				auto current_time = std::chrono::system_clock::now().time_since_epoch().count();
				std::cout << pktsPublished << " - " << current_time - prev_time_published << std::endl;
                prev_time_published = current_time;
			}

			pkt->timeStamp = clock();
			//std::cout << "before processPubPkt" << std::endl;
			this_engine->processPubPkt(pkt);
			//std::cout << "after processPubPkt" << std::endl;
			traceEvent(100, false);
			pthread_mutex_lock(packetQueueMutex);
			packetQueue.erase(packetQueue.begin());
			pthread_mutex_unlock(packetQueueMutex);
		}
	}
}

#define SINGLE_RULE
void testEngine(){
	pthread_mutex_init(packetQueueMutex, NULL);
  std::cout << "testEngine" << std::endl;
	this_engine = new TRexEngine(number_threads);
	this_engine->finalize();

#ifdef SINGLE_RULE
  std::cout << "SINGLE_RULE" << std::endl;
  RuleR0 testRule;
	this_engine->processRulePkt(testRule.buildRule());
  auto testPackets = testRule.buildPublication();
  allPackets.insert(allPackets.end(), testPackets.begin(), testPackets.end());
  ResultListener* listener= new TestResultListener(testRule.buildSubscription());
  this_engine->addResultListener(listener);
#else
  RuleR1 testRule;
	for (int i = 0; i < 20; i+=2) {
	    for (int j = 0; j < 100; j++) {
	        int fire_event = ((i+1)*(j+1))+101;
            this_engine->processRulePkt(testRule.buildRule(i, i + 1, fire_event, j%10));
            auto testPackets = testRule.buildPublication(i, i + 1, j%10);
            allPackets.insert(allPackets.end(), testPackets.begin(), testPackets.end());
            ResultListener* listener= new TestResultListener(testRule.buildSubscription(fire_event));
            this_engine->addResultListener(listener);
        }
	}
#endif

	boost::thread th{PublishPackets};
	t.async_wait(&HandlePubPacket);
	io.run();

	/* Expected output: complex event should be created by T-Rex and published
	 * to the TestResultListener, which should print it to screen.
	 */
}

extern std::string trace_name = "";
extern bool doTrace;
int main(int argc, char* argv[]){
	bool test = false;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-trace"))
			doTrace = true;
		else if (!strcmp(argv[i], "-number-threads")) {
      ++i;
      number_threads = std::atoi(argv[i]);
    } else if (!strcmp(argv[i], "-test")) {
      test = true;
    } else if (!strcmp(argv[i], "-trace-name")) {
		  ++i;
		  trace_name = argv[i];
		}
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
