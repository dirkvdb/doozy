//    Copyright (C) 2014 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "doozydeviceinterface.h"

#include "server/server.h"
#include "server/serversettings.h"

#include "renderer/renderer.h"
#include "renderer/renderersettings.h"

namespace doozy
{
namespace DeviceFactory
{

std::unique_ptr<IDevice> createDevice(const std::string& deviceType, const std::string& configFile)
{
    if (deviceType == "server")
    {
        ServerSettings settings;
        settings.loadFromFile(configFile);
        return std::unique_ptr<IDevice>(new Server(settings));
    }
    else if (deviceType == "renderer")
    {
        RendererSettings settings;
        settings.loadFromFile(configFile);
        return std::unique_ptr<IDevice>(new Renderer(settings));
    }

    throw std::runtime_error("Unsupported device type: " + deviceType);
}
    
}
}
