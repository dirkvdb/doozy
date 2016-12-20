add_library(doozycommon
    doozydeviceinterface.h
    settings.h settings.cpp
    devicedescriptions.h
)

target_include_directories(doozycommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(doozycommon utils)
