# FindWebSocketpp.cmake - 查找WebSocket++库

find_path(WEBSOCKETPP_INCLUDE_DIR
    NAMES websocketpp/version.hpp
    HINTS
        ${WEBSOCKETPP_ROOT}
        ${CMAKE_SOURCE_DIR}/third_party/websocketpp
        $ENV{WEBSOCKETPP_ROOT}
    PATHS
        /usr/local/include
        /usr/include
    DOC "WebSocket++ include directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Websocketpp
    REQUIRED_VARS WEBSOCKETPP_INCLUDE_DIR
)

if(WEBSOCKETPP_FOUND AND NOT TARGET Websocketpp::Websocketpp)
    add_library(Websocketpp::Websocketpp INTERFACE IMPORTED)
    set_target_properties(Websocketpp::Websocketpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${WEBSOCKETPP_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(WEBSOCKETPP_INCLUDE_DIR)