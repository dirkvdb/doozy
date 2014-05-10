namespace cpp doozy.rpc

struct Device
{
    1: required string name;
    2: required string udn;
}

struct Item
{
    1: required string id;
    2: required string title;
}

struct DeviceResponse
{
    1: required list<Device> devices;
}

struct BrowseRequest
{
    1: required string udn;
    2: required string containerid;
}

struct BrowseResponse
{
    1: required list<Item> items;
}

service ControlPoint
{
    DeviceResponse GetRenderers();
    DeviceResponse GetServers();
    BrowseResponse Browse(1:BrowseRequest req);
}