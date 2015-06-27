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

#include <array>
#include <stdexcept>

namespace doozy
{

using namespace utils;

static int cecLog(void*, const CEC::cec_log_message message)
{
    utils::log::debug("CEC: {}", message.message);
    return 0;
}

CecControl::CecControl(std::string device)
: m_cec(nullptr)
{
    snprintf(m_config.strDeviceName, 13, "Doozy");

    m_callbacks.CBCecLogMessage = &cecLog;
    m_config.callbacks = &m_callbacks;
    m_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);

    m_cec = reinterpret_cast<CEC::ICECAdapter*>(CECInitialise(&m_config));

    if (m_cec == nullptr)
    {
        throw std::runtime_error("Failed to initialize CEC adapter");
    }

    m_cec->InitVideoStandalone();

    if (device.empty())
    {
        std::array<CEC::cec_adapter_descriptor, 10> devices;
        auto count = m_cec->DetectAdapters(devices.data(), devices.size());

        if (count <= 0)
        {
            CECDestroy(m_cec);
            throw std::runtime_error("No CEC adapters found");
        }

        device = devices[0].strComName;
    }

    if (!m_cec->Open(device.c_str()))
    {
        CECDestroy(m_cec);
        throw std::runtime_error(fmt::format("Failed to open CEC connection on {}", device));
    }

    log::info("Opened CEC connection on {}", device);
}

CecControl::~CecControl()
{
    m_cec->Close();
    CECDestroy(m_cec);
}

void CecControl::turnOn()
{
    auto status = m_cec->GetDevicePowerStatus(CEC::CECDEVICE_AUDIOSYSTEM);
    log::info("Power status: {}", m_cec->ToString(status));
    if (status == CEC::CEC_POWER_STATUS_ON || status == CEC::CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY)
    {
        return;
    }

    if (!m_cec->PowerOnDevices(CEC::CECDEVICE_AUDIOSYSTEM))
    {
        log::error("Failed to turn on CEC device");
    }
}

void CecControl::standBy()
{
    if (!m_cec->StandbyDevices(CEC::CECDEVICE_AUDIOSYSTEM))
    {
        log::error("Failed to put CEC device in stand by");
    }
}

void CecControl::setActiveSource()
{
    if (!m_cec->SetActiveSource())
    {
        log::error("Failed to set CEC device as active");
    }
}

bool CecControl::isActiveSource()
{
    return m_cec->IsLibCECActiveSource();
}

}
