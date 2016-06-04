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
#include "doozyconfig.h"

#if DOOZY_SERVER
#include "server/server.h"
#include "server/serversettings.h"
#endif

#if DOOZY_RENDERER
#include "renderer/mediarendererdevice.h"
#endif

#include "utils/log.h"

namespace doozy
{
namespace DeviceFactory
{

using namespace utils;

std::unique_ptr<IDevice> createDevice(const std::string& deviceType, const std::string& configFile)
{
#if DOOZY_SERVER
    if (deviceType == "server")
    {
        ServerSettings settings;
        settings.loadFromFile(configFile);
        return std::make_unique<Server>(settings);
    }
#endif
#if DOOZY_RENDERER
    if (deviceType == "renderer")
    {
        RendererSettings settings;
        settings.loadFromFile(configFile);
        return std::make_unique<MediaRendererDevice>(settings);
    }
#endif

    throw std::runtime_error("Unsupported device type: " + deviceType);
}

}
}
