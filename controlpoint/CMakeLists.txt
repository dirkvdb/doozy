add_definitions("-DRAPIDJSON_HAS_STDSTRING")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(doozycontrolpoint
    doozycontrolpoint.h doozycontrolpoint.cpp
)

source_group(generated REGULAR_EXPRESSION gen-cpp/)

target_link_libraries(doozycontrolpoint upnpframework)
