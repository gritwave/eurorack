project(etl-audio)

add_library(etl-audio INTERFACE)
add_library(etl::audio ALIAS etl-audio)
target_include_directories(etl-audio INTERFACE ${PROJECT_SOURCE_DIR})
