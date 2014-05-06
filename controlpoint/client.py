

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from genpy.controlpoint import ControlPoint

# Make socket
transport = TSocket.TSocket('localhost', 9090)

# Buffering is critical. Raw sockets are very slow
transport = TTransport.TBufferedTransport(transport)

# Wrap in a protocol
protocol = TBinaryProtocol.TBinaryProtocol(transport)

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
