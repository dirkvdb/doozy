var Doozy = (function() {
    var _transport = new Thrift.Transport("http://localhost:9090");
    var _protocol  = new Thrift.Protocol(_transport);
    var _client    = new ControlPointClient(_protocol)

    function Doozy(){};

    function adddevice(combo, name) {
        var list = document.getElementById(combo);
        var entry = document.createElement('li');

        var a = document.createElement("a");
        a.appendChild(document.createTextNode(name));
        a.href = "#";
        entry.appendChild(a);
        list.appendChild(entry);
    }

    Doozy.prototype.getservers = function() {
        try {
            console.info("Get Servers");
            servers = _client.GetServers(function(resp) {
                for (var i = 0; i < resp.devices.length; ++i)
                {
                    console.info("Server: " + resp.devices[i].name);
                    adddevice("serverlist", resp.devices[i].name);
                }
            });
        } catch(ouch){
            console.error("Failed to get servers: " + ouch);
        }
    };

    Doozy.prototype.getrenderers = function () {
        try {
            console.info("Get Renderers");
            servers = _client.GetRenderers(function(resp) {
                for (var i = 0; i < resp.devices.length; ++i)
                {
                    console.info("Renderer: " + resp.devices[i].name);
                    adddevice("rendererlist", resp.devices[i].name);
                }
            });
        } catch(ouch){
            console.error("Failed to get renderers: " + ouch);
        }
    }

    return Doozy;

})();
