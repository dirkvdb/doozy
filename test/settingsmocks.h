// Copyright (c) 1995-2014 by FEI Company
// All rights reserved. This file includes confidential and
// proprietary information of FEI Company.

#pragma once

#include "gmock/gmock.h"

#include "server/serversettings.h"
#include "renderer/renderersettings.h"

namespace doozy
{
namespace test
{

class ServerSettingsMock : public ServerSettings
{
public:
    MOCK_CONST_METHOD0(getFriendlyName, std::string());
    MOCK_CONST_METHOD0(getUdn, std::string());
    MOCK_CONST_METHOD0(getDatabaseFilePath, std::string());
    MOCK_CONST_METHOD0(getLibraryPath, std::string());
    MOCK_CONST_METHOD0(getAlbumArtFilenames, std::vector<std::string>());
};

class RendererSettingsMock : public RendererSettings
{
public:
    MOCK_CONST_METHOD0(getFriendlyName, std::string());
    MOCK_CONST_METHOD0(getUdn, std::string());
    MOCK_CONST_METHOD0(getAudioOutput, std::string());
    MOCK_CONST_METHOD0(getAudioDevice, std::string());
};

}
}
