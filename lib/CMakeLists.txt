project(gw-libs)

add_library(gw_core INTERFACE)
add_library(gw::core ALIAS gw_core)
target_include_directories(gw_core INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(gw_core INTERFACE tetl::etl)

add_library(gw_audio INTERFACE)
add_library(gw::audio ALIAS gw_audio)
target_include_directories(gw_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(gw_audio INTERFACE gw::core)

add_library(gw_eurorack INTERFACE)
add_library(gw::eurorack ALIAS gw_eurorack)
target_include_directories(gw_eurorack INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(gw_eurorack INTERFACE gw::audio)
