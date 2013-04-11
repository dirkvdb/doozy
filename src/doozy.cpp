//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "doozy.h"

#include "devicedescriptions.h"
#include "mediarendererdevice.h"

#include "utils/log.h"
#include "utils/stringoperations.h"

#include "upnp/upnpwebserver.h"
#include "upnp/upnphttpreader.h"

#include "audio/audioreaderfactory.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

void Doozy::run()
{
    try
    {
        // make sure we can read http urls
        audio::ReaderFactory::registerBuilder(std::unique_ptr<IReaderBuilder>(new upnp::HttpReaderBuilder()));
    
        m_Client.initialize();
        
        std::string friendlyName = "Doozy";
        std::string udn = "uuid:356a6e90-8e58-11e2-9e96-0800200c9a66";
        
        std::string description = format(g_mediaRendererDevice.c_str(), m_Client.getIpAddress(), m_Client.getPort(),
                                                                        friendlyName, udn);
        
        upnp::WebServer webserver("/opt/");
        
        webserver.addVirtualDirectory("Doozy");
        addServiceFileToWebserver(webserver, "RenderingControlDesc.xml", g_rendererControlService);
        addServiceFileToWebserver(webserver, "ConnectionManagerDesc.xml", g_connectionManagerService);
        addServiceFileToWebserver(webserver, "AVTransportDesc.xml", g_avTransportService);
        
        MediaRendererDevice dev(udn, description, 180);
        dev.start();
        
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock);
        
        dev.stop();
        webserver.removeVirtualDirectory("Doozy");
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
    
    m_Client.destroy();
}
    
void Doozy::stop()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Condition.notify_all();
}
    
void Doozy::addServiceFileToWebserver(upnp::WebServer& webserver, const std::string& filename, const std::string& fileContents)
{
    webserver.addFile("Doozy", filename, "text/xml", fileContents);
}
    
}