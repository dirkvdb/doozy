// Copyright (c) 1995-2014 by FEI Company
// All rights reserved. This file includes confidential and
// proprietary information of FEI Company.

#pragma once

#include <zmq.hpp>

#include <google/protobuf/service.h>

namespace doozy
{

class PublishChannel : public google::protobuf::RpcChannel
{
public:
    PublishChannel(zmq::context_t& context, const std::string& ip, uint32_t port);

    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) override;

private:
    zmq::socket_t           m_socket;
};

}
