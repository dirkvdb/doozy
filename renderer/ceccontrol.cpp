//    Copyright (C) Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "ceccontrol.h"

#include "utils/log.h"
#include "utils/cppcompat.h"

#include <cec.h>
#include <array>
#include <stdexcept>

namespace doozy
{

using namespace utils;

int cecLog(void*, const CEC::cec_log_message message)
{
    utils::log::debug("CEC: {}", message.message);
    return 0;
}

CecControl::CecControl()
: m_cec(nullptr)
{
    CEC::libcec_configuration config;
    CEC::ICECCallbacks callbacks;
    config.Clear();
    callbacks.Clear();

    callbacks.CBCecLogMessage = &cecLog;
    config.callbacks = &callbacks;

    m_cec = reinterpret_cast<CEC::ICECAdapter*>(CECInitialise(&config));

    if (m_cec == nullptr)
    {
        throw std::runtime_error("Failed to initialize CEC adapter");
    }

    std::array<CEC::cec_adapter_descriptor, 10> devices;
    auto count = m_cec->DetectAdapters(devices.data(), devices.size());

    if (count == 0)
    {
        CECDestroy(m_cec);
        throw std::runtime_error("No CEC adapters found");
    }

    if (!m_cec->Open(devices[0].strComName))
    {
        CECDestroy(m_cec);
        throw std::runtime_error(fmt::format("Failed to open CEC connection on {}", devices[0].strComName));
    }

    log::info("Opened CEC connection on {}", devices[0].strComName);
}

CecControl::~CecControl()
{
    m_cec->Close();
    CECDestroy(m_cec);
}

void CecControl::TurnOn()
{
    if (!m_cec->PowerOnDevices(CEC::CECDEVICE_AUDIOSYSTEM))
    {
        log::error("Failed to turn on CEC device");
    }
}

void CecControl::StandBy()
{
    if (!m_cec->StandbyDevices(CEC::CECDEVICE_AUDIOSYSTEM))
    {
        log::error("Failed to put CEC device in stand by");
    }
}

}
