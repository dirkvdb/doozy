#include <string>
#include <future>
#include <zmq.hpp>

#include "utils/log.h"
#include "rpccall.pb.h"

using namespace utils;

namespace doozy
{

template <class Service>
class Worker
{
public:
    Worker(zmq::context_t& context, Service& service, const std::string& instanceName)
    : m_socket(context, ZMQ_REP)
    , m_service(service)
    , m_name(instanceName)
    {
        m_thread = std::async(std::launch::async, [this] () { run(); });
    }
    
    ~Worker()
    {
        m_thread.wait();
    }
    
    void run()
    {
        try
        {
            m_socket.connect("inproc://#1");
            
            for (;;)
            {
                log::info("Worker running: %s", m_name);
            
                //  Wait for next request from client
                zmq::message_t message;
                m_socket.recv(&message);

                log::info("[%s] Request received", m_name);

                doozy::proto::RPCRequest req;
                if (!req.ParseFromArray(message.data(), static_cast<int>(message.size())))
                {
                    log::error("Invalid request received");
                    break;
                }

                if (m_service.descriptor()->full_name() == req.service())
                {
                    auto method = m_service.GetDescriptor()->FindMethodByName(req.method());
                    if (method == nullptr)
                    {
                        log::error("Invalid method name received");
                        break;
                    }

                    auto reqProto = m_service.GetRequestPrototype(method).New();
                    auto resProto = m_service.GetResponsePrototype(method).New();
                    reqProto->ParseFromString(req.payload());

                    m_service.CallMethod(method, nullptr, reqProto, resProto, nullptr);

                    doozy::proto::RPCResponse rpcResp;
                    rpcResp.set_protobuf(resProto->SerializeAsString());
                    auto str = rpcResp.SerializeAsString();
                    m_socket.send(str.data(), str.size());
                }
            }
        }
        catch (zmq::error_t& e)
        {
            log::error("ZMQ error: %s (%d)", e.what(), e.num());
        }
    }
    
private:
    zmq::socket_t m_socket;
    Service& m_service;
    std::string m_name;
    std::future<void> m_thread;
};

}
