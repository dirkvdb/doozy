#include <iostream>
#include <sstream>
#include <zmq.hpp>

#include "utils/log.h"

#include "doozycontrolpoint.h"
#include "publishchannel.h"
#include "zmqworker.h"

using namespace utils;

int main(int argc, char** argv)
{
    utils::log::info("Control point");
    
    try
    {
        zmq::context_t context(1);
        zmq::socket_t frontend(context, ZMQ_ROUTER);
        zmq::socket_t backend(context, ZMQ_DEALER);
        
        doozy::PublishChannel pubChannel(context, "*", 9091);
        doozy::ControlPoint cp(pubChannel);
        
        frontend.bind("tcp://*:9090");
        backend.bind("inproc://#1");
        
        doozy::Worker<doozy::ControlPoint> worker1(context, cp, "worker1");
        doozy::Worker<doozy::ControlPoint> worker2(context, cp, "worker2");
        
        zmq::proxy(frontend, backend, nullptr);

        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        utils::log::error(e.what());
        return EXIT_FAILURE;
    }
}
