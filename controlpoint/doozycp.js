var Doozy = (function() {
    var _transport = new Thrift.Transport("http://localhost:9090");
    var _protocol  = new Thrift.Protocol(_transport);
    var _client    = new ControlPointClient(_protocol)

    function Doozy(){};

    function adddevice(combo, dev) {
        var list = document.getElementById(combo);
        var entry = document.createElement('li');

        var a = document.createElement("a");
        a.appendChild(document.createTextNode(dev.name));
        a.href = "#";
        a.className = "server";
        a.dataset.udn= dev.udn;
        entry.appendChild(a);
        list.appendChild(entry);
    }

    function clearitems() {
        $("#upnpitems").empty();
    }

    function additem(item) {
        var itemDiv = $("#upnpitemtemplate").clone();
        itemDiv.attr("id", item.id)
        itemDiv.appendTo("#upnpitems");
        itemDiv.find(".caption").html('<h3>' + item.title + '</h3>');

        var src = "";        
        if (item.thumbnailurl) {
            src = item.thumbnailurl;
        }
        else if (item.itemclass == ItemClass['Container']) {
            src = "images/container.png";
        }

        itemDiv.find(".thumbnailimg").attr("src", src);
        itemDiv.show();
    }

    Doozy.prototype.getservers = function() {
        try {
            console.info("Get Servers");
            _client.GetServers(function(resp) {
                for (var i = 0; i < resp.devices.length; ++i)
                {
                    adddevice("serverlist", resp.devices[i]);
                }
            });
        } catch(ouch) {
            console.error("Failed to get servers: " + ouch);
        }
    };

    Doozy.prototype.getrenderers = function () {
        try {
            console.info("Get Renderers");
            _client.GetRenderers(function(resp) {
                for (var i = 0; i < resp.devices.length; ++i)
                {
                    adddevice("rendererlist", resp.devices[i]);
                }
            });
        } catch(ouch) {
            console.error("Failed to get renderers: " + ouch);
        }
    }

    Doozy.prototype.browse = function (serverudn, containerid) {
        try {
            console.info("Browse:" + serverudn + ' (' + containerid + ')');
            var req = new BrowseRequest();
            req.udn = serverudn;
            req.containerid = containerid;
            _client.Browse(req, function(resp) {
                clearitems();
                for (var i = 0; i < resp.items.length; ++i)
                {
                    console.info("item: " + resp.items[i].title);
                    additem(resp.items[i]);
                }
            });
        } catch(ouch) {
            console.error("Failed to get items: " + ouch);
        }
    }

    return Doozy;

})();
