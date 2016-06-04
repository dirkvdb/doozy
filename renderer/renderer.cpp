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

#include "renderer.h"

#include "common/settings.h"
#include "devicedescriptions.h"
#include "mediarendererdevice.h"

#include "utils/log.h"
#include "utils/format.h"
#include "utils/readerfactory.h"
#include "utils/stringoperations.h"

#include "upnp/upnpwebserver.h"
#include "upnp/upnp.http.reader.h"

namespace doozy
{

using namespace utils;
using namespace std::chrono_literals;

Renderer::Renderer(RendererSettings& settings)
: m_settings(settings)
, m_stop(false)
{
    // make sure we can read http urls
    ReaderFactory::registerBuilder(std::make_unique<upnp::http::ReaderBuilder>());
}

Renderer::~Renderer() = default;

void Renderer::start()
{
    try
    {
        m_stop = false;

        auto udn                = "uuid:" + m_settings.getUdn();
        auto friendlyName       = m_settings.getFriendlyName();
        auto audioOutput        = m_settings.getAudioOutput();
        auto audioDevice        = m_settings.getAudioDevice();
        auto cecDevice          = m_settings.getCecDevice();

        // TODO: rendererdevice knows the ip
        auto description        = fmt::format(g_mediaRendererDevice.c_str(), "", "", friendlyName, udn);


        auto advertiseInterval  = 180s;

        log::info("FriendlyName = {}", friendlyName);
        log::info("AudioOutput = {}", audioOutput);
        log::info("AudioDevice = {}", audioDevice);

        //upnp::WebServer webserver("/opt/");

        //webserver.addVirtualDirectory("Doozy");
        //addServiceFileToWebserver(webserver, "RenderingControlDesc.xml", g_rendererControlService);
        //addServiceFileToWebserver(webserver, "ConnectionManagerDesc.xml", g_connectionManagerService);
        //addServiceFileToWebserver(webserver, "AVTransportDesc.xml", g_avTransportService);

        MediaRendererDevice dev(udn, description, advertiseInterval, audioOutput, audioDevice, cecDevice);
        dev.start();

        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] () { return m_stop == true; });

        dev.stop();
        //webserver.removeVirtualDirectory("Doozy");
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}

void Renderer::stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stop = true;
    m_condition.notify_all();
}

void Renderer::addServiceFileToWebserver(upnp::WebServer& webserver, const std::string& filename, const std::string& fileContents)
{
    webserver.addFile("Doozy", filename, "text/xml", fileContents);
}

}
