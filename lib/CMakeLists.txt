project(ta-libs)

add_library(ta_core INTERFACE)
add_library(ta::core ALIAS ta_core)
target_include_directories(ta_core INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(ta_core INTERFACE tetl::etl)

add_library(ta_audio INTERFACE)
add_library(ta::audio ALIAS ta_audio)
target_include_directories(ta_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(ta_audio INTERFACE ta::core)

add_library(ta_eurorack STATIC)
add_library(ta::eurorack ALIAS ta_eurorack)
target_include_directories(ta_eurorack PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(ta_eurorack PUBLIC ta::audio)
target_sources(ta_eurorack
    PUBLIC
        "ta/eurorack/hades.cpp"
        "ta/eurorack/hades.hpp"
)
