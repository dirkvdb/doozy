#include <iostream>
#include <sstream>

#include "utils/log.h"

#include "doozycontrolpoint.h"

#include <boost/make_shared.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

int main(int argc, char** argv)
{
    utils::log::info("Control point");
    
    const int port = 9090;
    auto handler = boost::make_shared<doozy::ControlPoint>();
    auto processor = boost::make_shared<doozy::rpc::ControlPointProcessor>(handler);
    auto serverTransport = boost::make_shared<TServerSocket>(port);
    auto transportFactory = boost::make_shared<TBufferedTransportFactory>();
    auto protocolFactory = boost::make_shared<TBinaryProtocolFactory>();
    
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();

    return 0;
}
