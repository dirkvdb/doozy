find_path(RAPIDJSON_INCLUDE_DIR
    NAMES rapidjson/rapidjson.h
)

if (NOT RAPIDJSON_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find rapid json header")
else ()
    message(STATUS "RapidJson include: ${RAPIDJSON_INCLUDE_DIR}")
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

target_compile_definitions(doozycontrolpoint PRIVATE RAPIDJSON_HAS_STDSTRING)
target_link_libraries(doozycontrolpoint upnpframework)
