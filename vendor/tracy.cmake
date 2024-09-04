if (MLN_USE_TRACY)
    add_definitions(-DTRACY_ENABLE)
    add_definitions(-DMLN_TRACY_ENABLE)
else()
    return()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/tracy)
add_library(TracyClient ${CMAKE_CURRENT_LIST_DIR}/tracy/tracy/TracyClient.cpp)

set_target_properties(
    TracyClient
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "Tracy profiler"
        INTERFACE_MAPLIBRE_URL "https://github.com/wolfpld/tracy.git"
        INTERFACE_MAPLIBRE_AUTHOR "Bartosz Taudul"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/tracy/LICENSE
)

