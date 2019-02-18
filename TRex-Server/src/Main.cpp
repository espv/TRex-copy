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

void testEngine(){
	TRexEngine engine(number_threads);
    engine.finalize();
	RuleR1 testRule;

	engine.processRulePkt(testRule.buildRule());

	ResultListener* listener= new TestResultListener(testRule.buildSubscription());
	engine.addResultListener(listener);

	vector<PubPkt*> pubPkts= testRule.buildPublication();
	for (vector<PubPkt*>::iterator it= pubPkts.begin(); it != pubPkts.end(); it++){
		engine.processPubPkt(*it);
	}
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
	    else if (!strcmp(argv[i], "-number-threads"))
	        number_threads = std::atoi(argv[i+1]);
	    else if (!strcmp(argv[i], "-test"))
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
