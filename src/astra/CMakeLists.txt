project(astra)

add_executable(astra)

target_sources(astra
    PRIVATE
        main.cpp
)

target_link_libraries(astra
    PRIVATE
        daisy
        mc::audio
)

set_target_properties(astra PROPERTIES
    SUFFIX
        ".elf"
    LINK_DEPENDS
        ${LINKER_SCRIPT}
)

target_link_options(astra PUBLIC
    -T ${LINKER_SCRIPT}
    -Wl,-Map=astra.map,--cref
    -Wl,--check-sections
    -Wl,--unresolved-symbols=report-all
    -Wl,--warn-common
    -Wl,--warn-section-align
    -Wl,--print-memory-usage
)

add_custom_command(TARGET astra POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O ihex -S $<TARGET_FILE:astra> $<TARGET_FILE_BASE_NAME:astra>.hex
    BYPRODUCTS
        astra.hex
    COMMENT
        "Generating HEX image"
    VERBATIM
)

add_custom_command(TARGET astra POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O binary -S $<TARGET_FILE:astra> $<TARGET_FILE_BASE_NAME:astra>.bin
    WORKING_DIRECTORY
        $<TARGET_FILE_DIR:astra>
    BYPRODUCTS
        astra.bin
    COMMENT
        "Generating binary image"
    VERBATIM
)
