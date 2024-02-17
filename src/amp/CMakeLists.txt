cmake_minimum_required(VERSION 3.23...3.27)
project(amp)

add_baremetal_executable(${PROJECT_NAME} ${LINKER_SCRIPT})
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE daisy gritwave::eurorack)
