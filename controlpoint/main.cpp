#include <iostream>
#include <sstream>
#include <zmq.hpp>

#include "utils/log.h"

#include "doozycontrolpoint.h"
#include "rpccall.pb.h"
#include "publishchannel.h"

int main(int argc, char** argv)
{
    utils::log::info("Control point");
    
    try
    {
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REP);
        
        socket.bind("tcp://*:9090");
    
        doozy::PublishChannel pubChannel(context, "*", 9091);
        doozy::ControlPoint cp(pubChannel);
        
        while (true)
        {
            //  Wait for next request from client
            zmq::message_t message;
            socket.recv(&message);
            std::cout << "Request received" << std::endl;

            doozy::proto::RPCRequest req;
            if (!req.ParseFromArray(message.data(), message.size()))
            {
                std::cerr << "Invalid request received" << std::endl;
                break;
            }

            std::cout << req.service() << "::" << req.method() << std::endl;
            if (cp.descriptor()->full_name() == req.service())
            {
                auto method = cp.GetDescriptor()->FindMethodByName(req.method());
                if (method == nullptr)
                {
                    std::cerr << "Invalid method name received" << std::endl;
                    break;
                }

                //auto controller = std::make_shared<RPCController>();

                auto reqProto = cp.GetRequestPrototype(method).New();
                auto resProto = cp.GetResponsePrototype(method).New();
                reqProto->ParseFromString(req.payload());

                //auto callback = google::protobuf::NewCallback(this, &Server::RequestFinished, resProto);
                cp.CallMethod(method, nullptr, reqProto, resProto, nullptr);

                doozy::proto::RPCResponse rpcResp;
                rpcResp.set_protobuf(resProto->SerializeAsString());
                auto str = rpcResp.SerializeAsString();
                socket.send(str.data(), str.size());
            }
        }

        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        utils::log::error(e.what());
        return EXIT_FAILURE;
    }
}
