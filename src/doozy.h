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


#ifndef DOOZY_H
#define DOOZY_H

#include <iostream>
#include <condition_variable>
#include <mutex>

#include "upnp/upnpclient.h"

namespace upnp
{
    class WebServer;
}

namespace doozy
{

class Doozy
{
public:
    void run(const std::string& configFile);
    void stop();
    
private:
    void addServiceFileToWebserver(upnp::WebServer& webserver, const std::string& filename, const std::string& fileContents);
    
    std::condition_variable     m_Condition;
    std::mutex                  m_Mutex;
    upnp::Client                m_Client;
};
    
}

#endif
