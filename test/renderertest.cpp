//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "doozydevicefactory.h"
#include "doozydeviceinterface.h"

#include "upnp/upnp.clientinterface.h"
#include "upnp/upnp.factory.h"
#include "upnp/upnp.devicescanner.h"
#include "upnp/upnp.mediarenderer.h"

#include <future>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace doozy
{
namespace test
{

class RendererTest : public Test
{
protected:
    RendererTest()
    : _renderer(doozy::DeviceFactory::createDevice("renderer", ""))
    , _client(upnp::factory::createClient())
    , _scanner(*_client, upnp::DeviceType(upnp::DeviceType::Type::MediaRenderer, 1))
    , _rendererClient(*_client)
    , _rendererThread(std::async(std::launch::async, [this] () { _renderer->start(""); }))
    {
        _client->initialize();
        _scanner.start();
        _scanner.refresh();
    }

    ~RendererTest()
    {
        _renderer->stop();
        _rendererThread.get();

        _scanner.stop();
        _client->uninitialize();
    }

    std::unique_ptr<IDevice> _renderer;
    std::unique_ptr<upnp::IClient> _client;
    upnp::DeviceScanner _scanner;
    upnp::MediaRenderer _rendererClient;
    std::future<void> _rendererThread;
};

TEST_F(RendererTest, Discover)
{
    bool found = false;
    const int maxAttempts = 10;
    std::shared_ptr<upnp::Device> dev;

    int attempts = 0;
    while (!found && attempts < maxAttempts)
    {
        auto devices = _scanner.getDevices();
        auto iter = std::find_if(devices.begin(), devices.end(), [] (auto& dev) {
            return dev->udn == "uuid:356a6e90-8e58-11e2-9e96-0800200c9a55";
        });

        found = iter != devices.end();

        if (found)
        {
            dev = *iter;
        }
        else
        {
            std::this_thread::sleep_for(100ms);
        }
    }

    ASSERT_TRUE(found);

    std::promise<upnp::Status> prom;
    auto fut = prom.get_future();
    _rendererClient.setDevice(dev, [&] (const upnp::Status& s) {
        prom.set_value(s);
    });

    EXPECT_TRUE(fut.get());
}

}
}
