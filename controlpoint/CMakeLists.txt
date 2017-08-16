add_definitions("-DRAPIDJSON_HAS_STDSTRING")

find_path(RAPIDJSON_INCLUDE_DIR
    NAMES rapidjson/writer.h
)

if (NOT RAPIDJSON_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find rapid json header")
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

add_library(doozycontrolpoint
    doozycontrolpoint.h doozycontrolpoint.cpp
)

source_group(generated REGULAR_EXPRESSION gen-cpp/)

target_link_libraries(doozycontrolpoint upnpframework)
