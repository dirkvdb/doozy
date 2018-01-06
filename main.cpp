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

#define _XOPEN_SOURCE

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "doozyconfig.h"
#include "settings.h"
#include "doozydeviceinterface.h"
#include "doozydevicefactory.h"

#include <cerrno>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>

using namespace utils;

namespace
{

std::unique_ptr<doozy::IDevice> g_deviceInstance;

void printUsage()
{
    std::vector<std::string> supportedDevices;

#if DOOZY_RENDERER
    supportedDevices.push_back("renderer");
#endif
#if DOOZY_SERVER
    supportedDevices.push_back("server");
#endif
#if DOOZY_CONTROLPOINT
    supportedDevices.push_back("controlpoint");
#endif

    std::cout   << "Usage: " PACKAGE_NAME " [options]" << std::endl << std::endl
                << "Options:" << std::endl
                << "  -t<s>   : device type (" << str::join(supportedDevices, "|") << ")" << std::endl
                << "  -i<s>   : network interface (" << "the name of the network interface to use" << ")" << std::endl
                << "  -f<s>   : config file" << std::endl
                << "  -d      : show debug logging" << std::endl
                << "  -h      : display this help" << std::endl;
}

std::string getOptArg(const char* arg)
{
    return arg == nullptr ? "" : str::trim(arg);
}

}

int main(int argc, char **argv)
{
#ifndef ANDROID
    if (!setlocale(LC_CTYPE, ""))
    {
        std::cerr << "Locale not specified. Check LANG, LC_CTYPE, LC_ALL" << std::endl;
        return 1;
    }
#endif

    int32_t option;
    bool debugLog = false;
    std::string configFile;
    std::string deviceType;
    std::string netInterface;

    while ((option = getopt(argc, argv, "f:t:i:d")) != -1)
    {
        switch (option)
        {
        case 't':
            deviceType = getOptArg(optarg);
            break;
        case 'd':
            debugLog = true;
            break;
        case 'i':
            netInterface = getOptArg(optarg);
            break;
        case 'f':
            configFile = getOptArg(optarg);
            break;
        case 'h':
            printUsage();
            return EXIT_SUCCESS;
        default:
            std::cerr << "Invalid arguments" << std::endl;
            printUsage();
            return EXIT_FAILURE;
        }
    }

    if (deviceType.empty())
    {
        std::cerr << "No device type provided" << std::endl;
        printUsage();
        return EXIT_FAILURE;
    }

    try
    {
        log::setFilter(debugLog ? log::Level::Debug : log::Level::Info);

        g_deviceInstance = doozy::DeviceFactory::createDevice(deviceType, configFile);
        log::info("Doozy {}", deviceType);
        g_deviceInstance->start(netInterface);
        log::info("Bye");

        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        log::error(e.what());
        return EXIT_FAILURE;
    }
}
