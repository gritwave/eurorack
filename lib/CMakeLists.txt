project(digitaldreams-audio)

add_library(digitaldreams_audio INTERFACE)
add_library(digitaldreams::audio ALIAS digitaldreams_audio)
target_include_directories(digitaldreams_audio INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(digitaldreams_audio INTERFACE tetl::etl)
