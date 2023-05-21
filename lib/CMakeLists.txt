project(mc-libs)

add_library(mc_core INTERFACE)
add_library(mc::core ALIAS mc_core)
target_include_directories(mc_core INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(mc_core INTERFACE tetl::etl)

add_library(mc_audio INTERFACE)
add_library(mc::audio ALIAS mc_audio)
target_include_directories(mc_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(mc_audio INTERFACE mc::core)
