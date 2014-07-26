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

#include <csignal>
#include <cerrno>
#include <string>
#include <cstring>
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>

#include "utils/log.h"
#include "common/settings.h"
#include "common/doozydeviceinterface.h"
#include "common/doozydevicefactory.h"
#include "common/crashhandler.h"

static bool set_signal_handlers();

using namespace utils;

static std::unique_ptr<doozy::IDevice> g_deviceInstance;

int main(int argc, char **argv)
{
#ifndef WIN32
    if (!set_signal_handlers())
    {
        return -1;
    }
#endif

    if (!setlocale(LC_CTYPE, "en_US.UTF-8"))
    {
        std::cerr << "Locale not specified. Check LANG, LC_CTYPE, LC_ALL" << std::endl;
        return 1;
    }
    
    int option;
    bool daemonize = false;
    std::string configFile;
    std::string deviceType;
    
    while ((option = getopt(argc, argv, "f:t:d")) != -1)
    {
        switch (option)
        {
        case 't':
            deviceType = optarg != nullptr ? optarg : "";
            break;
        case 'f':
            configFile = optarg != nullptr ? optarg : "";
            break;
        case 'd':
            daemonize = true;
            break;
        case '?':
        default:
            log::error("invalid arguments");
            //printUsage();
            return -1;
        }
    }

    try
    {
        g_deviceInstance = doozy::DeviceFactory::createDevice(deviceType, configFile);
        log::info("Doozy %s", deviceType);
        g_deviceInstance->start();
        log::info("Bye");

        return 0;
    }
    catch (std::exception& e)
    {
        log::error(e.what());
        return -1;
    }
}

static void sigterm(int signo)
{
    try
    {
        log::info("Sigterm %d", signo);
        g_deviceInstance->stop();
    }
    catch (std::exception& e)
    {
        log::error(e.what());
    }    
}

#ifndef WIN32
static bool set_signal_handlers()
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);

    if (sigaction(SIGINT, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGINT: %s", strerror(errno));
        return false;
    }

    sa.sa_flags = 0;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);

    if (sigaction(SIGQUIT, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGQUIT: %s", strerror(errno));
        return false;
    }

    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGTERM: %s", strerror(errno));
        return false;
    }
    
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = doozy::sigsegv;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    
    if (sigaction(SIGSEGV, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGSEGV: %s", strerror(errno));
        return false;
    }

    return true;
}

#endif
