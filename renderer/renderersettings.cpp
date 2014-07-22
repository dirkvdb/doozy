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

#include "renderersettings.h"
#include "audioconfig.h"

namespace doozy
{

static const std::string g_friendlyName = "FriendlyName";
static const std::string g_udn          = "UDN";
static const std::string g_audioOutput  = "AudioOutput";
static const std::string g_audioDevice  = "AudioDevice";

void RendererSettings::loadFromFile(const std::string& filepath)
{
    m_settings.loadFromFile(filepath);
}

std::string RendererSettings::getFriendlyName() const
{
    return m_settings.get(g_friendlyName, "Doozy");
}

std::string  RendererSettings::getUdn() const
{
    return m_settings.get(g_udn, "356a6e90-8e58-11e2-9e96-0800200c9a55");
}

std::string RendererSettings::getAudioOutput() const
{
    std::string fallback;

#ifdef HAVE_OPENAL
    fallback = "OpenAL";
#endif
#ifdef HAVE_ALSA
    fallback = "Alsa";
#endif
#ifdef HAVE_PULSE
    fallback = "PulseAudio";
#endif

    return m_settings.get(g_audioOutput, fallback);
}

std::string RendererSettings::getAudioDevice() const
{
    return m_settings.get(g_audioDevice, "Default");
}

}
