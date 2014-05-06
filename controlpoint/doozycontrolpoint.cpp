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

#include "doozycontrolpoint.h"

#include "utils/log.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

ControlPoint::ControlPoint()
: m_Cp(m_Client)
, m_RendererScanner(m_Client, upnp::Device::Type::MediaRenderer)
, m_ServerScanner(m_Client, upnp::Device::Type::MediaServer)
{
    run();
}

void ControlPoint::run()
{
    try
    {
        m_Client.initialize();
        m_RendererScanner.start();
        m_RendererScanner.refresh();
        m_ServerScanner.start();
        m_ServerScanner.refresh();
        m_Cp.activate();
        
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}
    
void ControlPoint::stop()
{
    m_RendererScanner.stop();
    m_ServerScanner.stop();
    m_Cp.deactivate();
    m_Client.destroy();
}
    
void ControlPoint::GetRenderers(rpc::DeviceResponse& response)
{
    log::info("Get renderers");
    
    auto devs = m_RendererScanner.getDevices();
    std::vector<rpc::Device> rpcDevs;
    std::transform(devs.begin(), devs.end(), std::back_inserter(rpcDevs), [&] (const std::pair<std::string, const std::shared_ptr<upnp::Device>>& dev) {
        rpc::Device rpcDev;
        rpcDev.__set_name(dev.second->m_FriendlyName);
        rpcDev.__set_udn(dev.second->m_UDN);
        return rpcDev;
    });

    response.__set_devices(rpcDevs);
}
    
void ControlPoint::GetServers(rpc::DeviceResponse& response)
{
    log::info("Get servers");
    
    auto devs = m_ServerScanner.getDevices();
    std::vector<rpc::Device> rpcDevs;
    std::transform(devs.begin(), devs.end(), std::back_inserter(rpcDevs), [&] (const std::pair<std::string, const std::shared_ptr<upnp::Device>>& dev) {
        rpc::Device rpcDev;
        rpcDev.__set_name(dev.second->m_FriendlyName);
        rpcDev.__set_udn(dev.second->m_UDN);
        return rpcDev;
    });
    
    response.__set_devices(rpcDevs);
}
    
}