

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.transport import THttpClient
from thrift.protocol import TJSONProtocol

from genpy.controlpoint import ControlPoint
from genpy.controlpoint import ttypes


transport = THttpClient.THttpClient('localhost', 9090, '/')

# Wrap in a protocol
protocol = TJSONProtocol.TJSONProtocol(transport)

# Create a client to use the protocol encoder
client = ControlPoint.Client(protocol)

# Connect!
transport.open()

renderers = client.GetRenderers()
print 'renderers:'
for r in renderers.devices:
    print r.name + ' (' + r.udn + ')'

servers = client.GetServers()
print 'servers:'
for r in servers.devices:
    print r.name + ' (' + r.udn + ')'

    req = ttypes.BrowseRequest()
    req.udn = r.udn
    req.containerid = "0"
    res = client.Browse(req)

    for item in res.items:
    	print item.title + ' (' + item.id + ')'
