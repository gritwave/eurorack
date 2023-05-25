project(mc-libs)

add_library(mc_core INTERFACE)
add_library(mc::core ALIAS mc_core)
target_include_directories(mc_core INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(mc_core INTERFACE tetl::etl)

add_library(mc_audio INTERFACE)
add_library(mc::audio ALIAS mc_audio)
target_include_directories(mc_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(mc_audio INTERFACE mc::core)

add_library(mc_eurorack STATIC)
add_library(mc::eurorack ALIAS mc_eurorack)
target_include_directories(mc_eurorack PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(mc_eurorack PUBLIC mc::audio)
target_sources(mc_eurorack
    PUBLIC
        "mc/eurorack/hades.cpp"
        "mc/eurorack/hades.hpp"
)
