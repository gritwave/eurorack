cmake_minimum_required(VERSION 3.23)
project(kyma LANGUAGES C CXX)

find_program(CCACHE ccache)
if (CCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBDAISY_DIR ${CMAKE_SOURCE_DIR}/3rd_party/libDaisy)
add_subdirectory(${LIBDAISY_DIR} libdaisy)
set(LINKER_SCRIPT ${LIBDAISY_DIR}/core/STM32H750IB_flash.lds)

add_executable(kyma)
target_sources(kyma PRIVATE src/kyma/main.cpp)

target_link_libraries(kyma
    PRIVATE
        daisy
)

set_target_properties(kyma PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(kyma PUBLIC
    -T ${LINKER_SCRIPT}
    -Wl,-Map=kyma.map,--cref
    -Wl,--check-sections
    -Wl,--unresolved-symbols=report-all
    -Wl,--warn-common
    -Wl,--warn-section-align
    -Wl,--print-memory-usage
)

# add_custom_command(TARGET kyma POST_BUILD
#     COMMAND
#         ${CMAKE_OBJCOPY}
#     ARGS
#         -O ihex -S kyma.elf kyma.hex
#     BYPRODUCTS
#         kyma.hex
#     COMMENT
#         "Generating HEX image"
#     VERBATIM
# )

add_custom_command(TARGET kyma POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O binary -S $<TARGET_FILE:kyma> $<TARGET_FILE_BASE_NAME:kyma>.bin
    WORKING_DIRECTORY
        $<TARGET_FILE_DIR:kyma>
    BYPRODUCTS
        kyma.bin
    COMMENT
        "Generating binary image"
    VERBATIM
)
