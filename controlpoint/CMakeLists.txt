add_definitions("-DRAPIDJSON_HAS_STDSTRING")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_LIBRARY(doozycontrolpoint
    doozycontrolpoint.h doozycontrolpoint.cpp
)

SOURCE_GROUP(generated REGULAR_EXPRESSION gen-cpp/)

TARGET_LINK_LIBRARIES(doozycontrolpoint upnpframework)
