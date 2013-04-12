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

std::string Settings::get(const std::string& setting, const std::string& defaultValue) const
{
    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        return iter->second;
    }

    return defaultValue;
}

int32_t Settings::getAsInt(const std::string& setting, int32_t defaultValue) const
{
    int32_t result = defaultValue;

    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        result = stringops::toNumeric<int32_t>(iter->second);
    }

    return result;
}

bool Settings::getAsBool(const std::string& setting, bool defaultValue) const
{
    bool result = defaultValue;

    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        std::string value = stringops::lowercase(iter->second);
        if (value == "true")
        {
            result = true;
        }
        else if (value == "false")
        {
            result = false;
        }
    }

    return result;
}

void Settings::getAsVector(const std::string& setting, std::vector<std::string>& array) const
{
	array.clear();
	std::string value = get(setting);

	if (!value.empty())
	{
		array = stringops::tokenize(value, ";");
		for (size_t i = 0; i < array.size(); ++i)
		{
			stringops::trim(array[i]);
		}
	}
}


void Settings::set(const std::string& setting, const std::string& value)
{
    m_Settings[setting] = value;
}

void Settings::set(const std::string& setting, int32_t value)
{
    set(setting, numericops::toString(value));
}

void Settings::set(const std::string& setting, bool value)
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
            log::warn("Warning: ignoring malformed line in config file: %d" + line);
            continue;
        }

        std::string setting = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        stringops::trim(setting);
        stringops::trim(value);
        m_Settings[setting] = value;
    }
}

void Settings::loadDefaultSettings()
{
#ifdef HAVE_OPENAL
    m_Settings["AudioOutput"]   = "OpenAL";
#endif
#ifdef HAVE_ALSA
    m_Settings["AudioOutput"]   = "Alsa";
#endif
#ifdef HAVE_PULSE
    m_Settings["AudioOutput"]   = "PulseAudio";
#endif

    m_Settings["AudioDevice"]   = "default";
    m_Settings["FriendlyName"]  = "Doozy";
    m_Settings["UDN"]           = "356a6e90-8e58-11e2-9e96-0800200c9a66";
}

}
