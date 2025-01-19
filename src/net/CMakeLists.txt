file(GLOB net_sources CONFIGURE_DEPENDS "*.c")
add_library(net STATIC ${net_sources})

add_custom_target(ping
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/ping.py 22307
)
