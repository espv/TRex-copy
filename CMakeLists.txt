cmake_minimum_required(VERSION 3.12)
project(TRexServerProject)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(TRex-Server)
include_directories(TRex-Server/src)
include_directories(TRex-Server/src/Connection)
include_directories(TRex-Server/src/Packet)
include_directories(TRex-Server/src/Server)
include_directories(TRex-Server/src/Test)
include_directories(TRex-Server/src/Util)
include_directories(TRex2-lib)
include_directories(TRex2-lib/src/Common)
include_directories(TRex2-lib/src/Communication)
include_directories(TRex2-lib/src/Engine)
include_directories(TRex2-lib/src/Engine/GPU)
include_directories(TRex2-lib/src/Marshalling)
include_directories(TRex2-lib/src/Packets)


# This custom target builds the entire T-Rex project in order to run TRex-Server/src/TRexServer.
# Before building TRex-Server, we must build TRex2-lib. However, because of how the TRex-Server Makefile is set up,
# it will always rebuild the link with TRex2-lib when the library has been changed, i.e., make install is executed in
# TRex2-lib. To prevent this, we will only run make install in TRex2-lib if a source file has changed in TRex2-lib,
# or, the output from running "make" is not "Nothing to be done for 'all'.
add_custom_target(TRexServer COMMAND ${PROJECT_SOURCE_DIR}/TRex-Server/custom_setup.sh
        CLION_EXE_DIR=${PROJECT_SOURCE_DIR}/TRex-Server/src)

add_custom_target(JustRunTRexServer
        CLION_EXE_DIR=${PROJECT_SOURCE_DIR}/TRex-Server/src)

add_custom_target(ConfigureDebugTRex COMMAND ${PROJECT_SOURCE_DIR}/TRex2-lib/run_debug_configure && ${PROJECT_SOURCE_DIR}/TRex-Server/run_debug_configure
        CLION_EXE_DIR=${PROJECT_SOURCE_DIR}/TRex2-Server/src)

add_custom_target(ConfigureProdTRex COMMAND ${PROJECT_SOURCE_DIR}/TRex2-lib/run_production_configure && ${PROJECT_SOURCE_DIR}/TRex-Server/run_production_configure
        CLION_EXE_DIR=${PROJECT_SOURCE_DIR}/TRex-Server/src)

set_target_properties(TRexServer PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(JustRunTRexServer PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ConfigureDebugTRex PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ConfigureProdTRex PROPERTIES LINKER_LANGUAGE CXX)

add_executable(TRexServerProject
        TRex2-lib/config.h
        TRex2-lib/src/Packets/JoinPkt.h
        TRex2-lib/src/Packets/PubPkt.h
        TRex2-lib/src/Packets/RulePkt.h
        TRex2-lib/src/Packets/RulePktValueReference.h
        TRex2-lib/src/Packets/AdvPkt.h
        TRex2-lib/src/Packets/SubPkt.h
        TRex2-lib/src/Packets/StaticValueReference.h
        TRex2-lib/src/Communication/SimpleRoutingTable.h
        TRex2-lib/src/Communication/TRexServer.h
        TRex2-lib/src/Communication/GenericRoutingTable.h
        TRex2-lib/src/Common/TimeMs.h
        TRex2-lib/src/Common/Funs.h
        TRex2-lib/src/Common/OpTree.h
        TRex2-lib/src/Common/Timer.h
        TRex2-lib/src/Common/Structures.h
        TRex2-lib/src/Common/CompositeEventTemplate.h
        TRex2-lib/src/Common/OpValueReference.h
        TRex2-lib/src/Common/Consts.h
        TRex2-lib/src/Marshalling/Marshaller.h
        TRex2-lib/src/Marshalling/Unmarshaller.h
        TRex2-lib/src/Engine/IndexingTableCommon.h
        TRex2-lib/src/Engine/StringConstraintIndex.h
        TRex2-lib/src/Engine/StacksRule.h
        TRex2-lib/src/Engine/ResultListener.h
        TRex2-lib/src/Engine/BoolConstraintIndex.h
        TRex2-lib/src/Engine/IndexingTable.h
        TRex2-lib/src/Engine/Stack.h
        TRex2-lib/src/Engine/CompositeEventGenerator.h
        TRex2-lib/src/Engine/FloatConstraintIndex.h
        TRex2-lib/src/Engine/GPU/GPUProcessorIf.h
        TRex2-lib/src/Engine/GPU/MemoryManager.h
        TRex2-lib/src/Engine/GPU/GPUProcessor.h
        TRex2-lib/src/Engine/GPU/GPUEngine.h
        TRex2-lib/src/Engine/GPU/CudaKernels.h
        TRex2-lib/src/Engine/NoConstraintIndex.h
        TRex2-lib/src/Engine/AbstractConstraintIndex.h
        TRex2-lib/src/Engine/IntConstraintIndex.h
        TRex2-lib/src/Engine/Engine.h
        TRex2-lib/src/Engine/TRexEngine.h
        TRex-Server/config.h
        TRex2-lib/src/Common/trace-framework.hpp
        TRex-Server/src/util.hpp
        TRex-Server/src/Util/Logging.hpp
        TRex-Server/src/Util/CharVector.hpp
        TRex-Server/src/Util/TRexUtils.hpp
        TRex-Server/src/Server/SOEPServer.hpp
        TRex-Server/src/Packet/BufferedPacketUnmarshaller.hpp
        TRex-Server/src/Packet/UnSubPkt.hpp
        TRex-Server/src/Packet/Pkt.hpp
        TRex-Server/src/Packet/PacketMarshaller.hpp
        TRex-Server/src/Packet/PingPkt.hpp
        TRex-Server/src/server.hpp
        TRex-Server/src/test.hpp
        TRex-Server/src/external.hpp
        TRex-Server/src/packet.hpp
        TRex-Server/src/Connection/RequestHandler.hpp
        TRex-Server/src/Connection/PubPktListener.hpp
        TRex-Server/src/Connection/Connection.hpp
        TRex-Server/src/Connection/ConnectionProxy.hpp
        TRex-Server/src/Connection/SubscriptionTable.hpp
        TRex-Server/src/Test/TestResultListener.hpp
        TRex-Server/src/Test/RuleR1.hpp
        TRex-Server/src/Test/RuleR0.hpp
        TRex-Server/src/Test/TestRule.hpp
        TRex-Server/src/connection.hpp)

set_target_properties(TRexServerProject PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE_FILES
        ./TRex2-lib/src/Common/trace-framework.cpp
        ./TRex-Server/src/Util/Logging.cpp
        ./TRex-Server/src/Util/TRexUtils.cpp
        ./TRex-Server/src/Main.cpp
        ./TRex-Server/src/Server/SOEPServer.cpp
        ./TRex-Server/src/Packet/BufferedPacketUnmarshaller.cpp
        ./TRex-Server/src/Packet/UnSubPkt.cpp
        ./TRex-Server/src/Packet/PingPkt.cpp
        ./TRex-Server/src/Packet/PacketMarshaller.cpp
        ./TRex-Server/src/Connection/Connection.cpp
        ./TRex-Server/src/Connection/ConnectionProxy.cpp
        ./TRex-Server/src/Connection/PubPktListener.cpp
        ./TRex-Server/src/Connection/RequestHandler.cpp
        ./TRex-Server/src/Connection/SubscriptionTable.cpp
        ./TRex-Server/src/Test/RuleR1.cpp
        ./TRex-Server/src/Test/RuleR0.cpp
        ./TRex-Server/src/Test/TestResultListener.cpp
        ./TRex-Server/src/Test/TestRule.cpp
        ./TRex2-lib/src/Packets/RulePktValueReference.cc
        ./TRex2-lib/src/Packets/JoinPkt.cc
        ./TRex2-lib/src/Packets/RulePkt.cc
        ./TRex2-lib/src/Packets/SubPkt.cc
        ./TRex2-lib/src/Packets/AdvPkt.cc
        ./TRex2-lib/src/Packets/StaticValueReference.cc
        ./TRex2-lib/src/Packets/PubPkt.cc
        ./TRex2-lib/src/Communication/TRexServer.cc
        ./TRex2-lib/src/Communication/SimpleRoutingTable.cc
        ./TRex2-lib/src/Common/CompositeEventTemplate.cc
        ./TRex2-lib/src/Common/OpTree.cc
        ./TRex2-lib/src/Common/Timer.cc
        ./TRex2-lib/src/Common/Funs.cc
        ./TRex2-lib/src/Common/TimeMs.cc
        ./TRex2-lib/src/Marshalling/Unmarshaller.cc
        ./TRex2-lib/src/Marshalling/Marshaller.cc
        ./TRex2-lib/src/Engine/TRexEngine.cc
        ./TRex2-lib/src/Engine/CompositeEventGenerator.cc
        ./TRex2-lib/src/Engine/Stack.cc
        ./TRex2-lib/src/Engine/NoConstraintIndex.cc
        ./TRex2-lib/src/Engine/IntConstraintIndex.cc
        ./TRex2-lib/src/Engine/StacksRule.cc
        ./TRex2-lib/src/Engine/GPU/GPUProcessor.cc
        ./TRex2-lib/src/Engine/GPU/GPUEngine.cc
        ./TRex2-lib/src/Engine/IndexingTable.cc
        ./TRex2-lib/src/Engine/FloatConstraintIndex.cc
        ./TRex2-lib/src/Engine/BoolConstraintIndex.cc
        ./TRex2-lib/src/Engine/StringConstraintIndex.cc
        ./TRex2-lib/src/Engine/AbstractConstraintIndex.cc)

add_executable(dummy ${SOURCE_FILES})
