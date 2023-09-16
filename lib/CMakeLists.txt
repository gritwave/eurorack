project(gw_eurorack)

add_library(gw_eurorack INTERFACE)
add_library(gw::eurorack ALIAS gw_eurorack)
target_include_directories(gw_eurorack INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(gw_eurorack INTERFACE tetl::etl)

if(CMAKE_CROSSCOMPILING)
    return()
endif()

add_executable(gw_eurorack_tests)
add_test(NAME gw_eurorack_tests COMMAND gw_eurorack_tests)
target_link_libraries(gw_eurorack_tests PRIVATE gw::eurorack)
target_sources(gw_eurorack_tests
    PRIVATE
        main_test.cpp
        gw/fft/fft_test.cpp
        gw/math/ilog2_test.cpp
        gw/math/ipow_test.cpp
)
