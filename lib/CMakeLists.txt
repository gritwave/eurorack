project(digitaldreams-libs)

add_library(digitaldreams_core INTERFACE)
add_library(digitaldreams::core ALIAS digitaldreams_core)
target_include_directories(digitaldreams_core INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(digitaldreams_core INTERFACE tetl::etl)

add_library(digitaldreams_audio INTERFACE)
add_library(digitaldreams::audio ALIAS digitaldreams_audio)
target_include_directories(digitaldreams_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(digitaldreams_audio INTERFACE digitaldreams::core)
