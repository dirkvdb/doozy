// Copyright (c) 1995-2014 by FEI Company
// All rights reserved. This file includes confidential and
// proprietary information of FEI Company.

#include "publishchannel.h"
#include "rpccall.pb.h"
#include "utils/log.h"

#include <sstream>

namespace doozy
{

using namespace google::protobuf;


PublishChannel::PublishChannel(zmq::context_t& context, const std::string& ip, uint32_t port)
: m_socket(context, ZMQ_PUB)
{
    utils::log::info("Bind socket");
    std::stringstream ss;
    ss << "tcp://" << ip << ":" << port;
    m_socket.bind(ss.str().c_str());
}

void PublishChannel::CallMethod(const MethodDescriptor* method, RpcController* controller, const Message* request, Message* response, Closure* done)
{
    proto::RPCRequest req;
    
    req.set_service(method->service()->full_name());
    req.set_method(method->name());
    req.set_payload(request->SerializeAsString());
    
    utils::log::info("Publish event on service: %s", req.service());

    auto requestString = req.SerializeAsString();
    std::string intf = method->service()->full_name();
    m_socket.send(intf.data(), intf.size(), ZMQ_SNDMORE);
    m_socket.send(requestString.data(), requestString.size());
    
    if (done)
    {
        done->Run();
    }
}

}
