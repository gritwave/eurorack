project(kyma)

add_baremetal_executable(${PROJECT_NAME} ${LINKER_SCRIPT})
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE daisy gw::eurorack)
