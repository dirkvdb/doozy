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

#include "rpccall.pb.h"
#include "controlpoint.pb.h"

#include "upnp/upnpclient.h"
#include "upnp/upnpwebserver.h"
#include "upnp/upnpdevicescanner.h"
#include "upnp/upnpcontrolpoint.h"
#include "upnp/upnpmediaserver.h"

namespace doozy
{

class PublishChannel;

class ControlPoint : public proto::ControlPoint
{
public:
    ControlPoint(PublishChannel& pubChannel);
    void run();
    void stop();
    
    void GetRenderers(google::protobuf::RpcController* controller, const proto::Void* request,
                      proto::DeviceResponse* response, google::protobuf::Closure* done);
    
    void GetServers(google::protobuf::RpcController* controller, const proto::Void* request,
                    proto::DeviceResponse* response, google::protobuf::Closure* done);
    
    void Browse(google::protobuf::RpcController* controller, const proto::BrowseRequest* request,
                proto::BrowseResponse* response, google::protobuf::Closure* done);
    
    void Play(google::protobuf::RpcController* controller, const proto::PlayRequest* request,
              proto::Void* response, google::protobuf::Closure* done);
    
    void GetRendererStatus(google::protobuf::RpcController* controller, const proto::Device* request,
                           proto::RendererStatus* response, google::protobuf::Closure* done);
    
private:
    upnp::Client                        m_Client;
    upnp::ControlPoint                  m_Cp;
    upnp::MediaServer                   m_MediaServer;
    upnp::DeviceScanner                 m_RendererScanner;
    upnp::DeviceScanner                 m_ServerScanner;
    std::unique_ptr<upnp::WebServer>    m_Webserver;
    
    proto::ControlPointEvents::Stub     m_events;
};
    
}

#endif
