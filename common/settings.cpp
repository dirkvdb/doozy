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

#include "settings.h"

#include <fstream>
#include <sstream>

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"

#include "audioconfig.h"

using namespace utils;

namespace doozy
{

std::string Settings::get(const std::string& setting) const
{
    return getSetting(setting);
}

std::string Settings::get(const std::string& setting, const std::string& defaultValue) const noexcept
{
    try
    {
        return get(setting);
    }
    catch (std::exception&)
    {
        return defaultValue;
    }
}

int32_t Settings::getAsInt(const std::string& setting) const
{
    return stringops::toNumeric<int32_t>(getSetting(setting));
}

int32_t Settings::getAsInt(const std::string& setting, int32_t defaultValue) const noexcept
{
    try
    {
        return getAsInt(setting);
    }
    catch (std::exception&)
    {
        return defaultValue;
    }
}


bool Settings::getAsBool(const std::string& setting) const
{
    std::string value = stringops::lowercase(getSetting(setting));
    if (value == "true")
    {
        return true;
    }
    else if (value == "false")
    {
        return false;
    }

    throw std::runtime_error(fmt::format("Invalid boolean value for setting {}: ({})", setting, value));
}

bool Settings::getAsBool(const std::string& setting, bool defaultValue) const noexcept
{
    try
    {
        return getAsBool(setting);
    }
    catch (std::exception&)
    {
        return defaultValue;
    }
}


std::vector<std::string> Settings::getAsVector(const std::string& setting) const
{
	std::vector<std::string> settings;
	std::string value = get(setting);

	if (!value.empty())
	{
		settings = stringops::tokenize(value, ";");
        std::for_each(settings.begin(), settings.end(), [] (std::string& s) {
            stringops::trim(s);
        });
	}

    return settings;
}

std::vector<std::string> Settings::getAsVector(const std::string& setting, const std::vector<std::string>& defaultValue) const noexcept
{
    try
    {
        return getAsVector(setting);
    }
    catch (std::exception&)
    {
        return defaultValue;
    }
}

void Settings::set(const std::string& setting, const char* value) noexcept
{
    set(setting, std::string(value));
}

void Settings::set(const std::string& setting, const std::string& value) noexcept
{
    m_Settings[setting] = value;
}

void Settings::set(const std::string& setting, int32_t value) noexcept
{
    set(setting, numericops::toString(value));
}

void Settings::set(const std::string& setting, bool value) noexcept
{
    set(setting, std::string(value ? "true" : "false"));
}

void Settings::loadFromFile(const std::string& filepath)
{
    std::ifstream settingsFile(filepath.c_str());
    if (!settingsFile.is_open())
    {
        log::info("No config file present, relying on default values");
        return;
    }

    std::string line;
    while (getline(settingsFile, line))
    {
        stringops::trim(line);
        if (line.empty())       continue;
        if (line.at(0) == '#')  continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
        {
            log::warn("Warning: ignoring malformed line in config file: {}" + line);
            continue;
        }

        std::string setting = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        stringops::trim(setting);
        stringops::trim(value);
        m_Settings[setting] = value;
    }
}

std::string Settings::getSetting(const std::string& setting) const
{
    auto iter = m_Settings.find(setting);
    if (iter == m_Settings.end())
    {
        throw std::runtime_error("No such setting: " + setting);
    }

    return iter->second;
}

void Settings::loadDefaultSettings()
{
#if HAVE_OPENAL
    m_Settings["AudioOutput"]       = "OpenAL";
#endif
#if HAVE_ALSA
    m_Settings["AudioOutput"]       = "Alsa";
#endif
#if HAVE_PULSE
    m_Settings["AudioOutput"]       = "PulseAudio";
#endif

    m_Settings["AudioDevice"]       = "default";
    m_Settings["FriendlyName"]      = "Doozy";
    m_Settings["UDN"]               = "356a6e90-8e58-11e2-9e96-0800200c9a66";
}

}
