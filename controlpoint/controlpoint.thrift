namespace cpp doozy.rpc

struct Device
{
    1: required string name;
    2: required string udn;
}

struct DeviceResponse
{
    1: list<Device> devices;
}

service ControlPoint
{
    DeviceResponse GetRenderers();
    DeviceResponse GetServers();
}