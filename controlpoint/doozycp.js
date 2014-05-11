/*jslint browser: true*/
/*global  $,Thrift,ControlPointClient*/

var Doozy = (function () {
    var _transport  = new Thrift.Transport("http://localhost:9090");
    var _protocol   = new Thrift.Protocol(_transport);
    var _client     = new ControlPointClient(_protocol);
    var _rootid     = '0';

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
        itemDiv.attr("id", item.id);
        itemDiv.appendTo("#upnpitems");
        itemDiv.find(".caption").html('<div class="itemtitle">' + item.title + '</div>');

        var src = "";
        if (item.thumbnailurl) {
            src = item.thumbnailurl;
        } else if (item.itemclass === ItemClass['Container']) {
            src = "images/container.png";
        }

        itemDiv.find(".thumbnailimg").attr("src", src);
        itemDiv.show();
    }

    Doozy.prototype.rootId = function () {
        return _rootid;
    }

    Doozy.prototype.getservers = function () {
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
    }

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

    Doozy.prototype.resetCrumbs = function () {
        $("#serverpath").children().find("li:gt(0)").remove();
    }

    Doozy.prototype.cleanCrumbs = function (activeCrumb) {
        var index = $("#serverpath li").index(activeCrumb);
        $("#serverpath").children().slice(index).detach();
        activeCrumb.addClass('active');

        //TODO: remove the lunk from the active crumb
    }

    Doozy.prototype.addCrumb = function (containerTitle, containerId) {
        var activeItem = $("#serverpath li.active");
        if (activeItem.length) {
            activeItem.contents()
                .filter(function () {
                    return this.nodeType === 3;
                })
                .wrap('<a href="#"></a>');
            activeItem.removeClass("active");
        }
        var crumb = document.createElement('li');
        crumb.className = 'active pathcrumb';
        crumb.dataset.id = containerId;
        crumb.dataset.title = containerTitle;
        crumb.appendChild(document.createTextNode(containerTitle));
        $("#serverpath").append(crumb);
    }

    Doozy.prototype.browse = function (serverudn, containerId) {
        try {
            console.info("Browse:" + serverudn + ' (' + containerId + ')');
            var req = new BrowseRequest();
            req.udn = serverudn;
            req.containerid = containerId;
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
