import controlpoint_pb2
import rpccall_pb2
import zmq
import google.protobuf.service as service


class ZMQChannel(service.RpcChannel):
    def __init__(self, ctxt):
        self.sock = ctxt.socket(zmq.REQ)
        self.sock.connect('tcp://localhost:9090')

    def CallMethod(self, method_descriptor, rpc_controller, request, response_class, done):
        req = rpccall_pb2.RPCRequest()
        req.service = method_descriptor.containing_service.full_name
        print req.service
        req.method = method_descriptor.name
        req.payload = request.SerializeToString()

        print 'send'
        self.sock.send(req.SerializeToString())
        print 'recv'
        res = self.sock.recv()
        print 'received'

        rpcResp = rpccall_pb2.RPCResponse()
        rpcResp.ParseFromString(res)

        resp = response_class()
        resp.ParseFromString(rpcResp.protobuf)
        return resp


class ControlPointEvents(controlpoint_pb2.ControlPointEvents):
    def DeviceDiscovered(self, rpc_controller, request, done):
        print 'device discovered: {0} ({1})'.format(request.name, request.udn)

if __name__ == "__main__":
    context = zmq.Context()
    void = rpccall_pb2.Void()
    cpEvents = ControlPointEvents()

    cp = controlpoint_pb2.ControlPoint_Stub(ZMQChannel(context))
    res = cp.GetRenderers(None, void, None)

    for dev in res.devices:
        print "device {0} ({1})".format(dev.name, dev.udn)

    #req = rpccall_pb2.RPCRequest()
    #req.service = 'doozy.proto.ControlPointEvents'
    #req.method = ''
    #req.payload = ''
    #print req.SerializeToString()

    #subsock = context.socket(zmq.SUB)
    #subsock.connect('tcp://localhost:9091')
    #subsock.setsockopt(zmq.SUBSCRIBE, 'doozy.proto.ControlPointEvents')
    #while True:
    #    req = rpccall_pb2.RPCRequest()
    #    rec = subsock.recv() # interface name
    #    rec = subsock.recv()
    #    req.ParseFromString(rec)
    #    print 'Service event: {0}::{1}'.format(req.service, req.method)
    #    method = cpEvents.GetDescriptor().FindMethodByName(req.method)
    #    aaa = cpEvents.GetRequestClass(method)()
    #    aaa.ParseFromString(req.payload)
    #    cpEvents.CallMethod(method, None, aaa, None)


