project(hades)

add_executable(hades)

target_sources(hades
    PRIVATE
        main.cpp
)

target_link_libraries(hades
    PRIVATE
        daisy
        mc::audio
)

set_target_properties(hades PROPERTIES
    SUFFIX
        ".elf"
    LINK_DEPENDS
        ${LINKER_SCRIPT}
)

target_link_options(hades PUBLIC
    -T ${LINKER_SCRIPT}
    -Wl,-Map=hades.map,--cref
    -Wl,--check-sections
    -Wl,--unresolved-symbols=report-all
    -Wl,--warn-common
    -Wl,--warn-section-align
    -Wl,--print-memory-usage
)

add_custom_command(TARGET hades POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O ihex -S $<TARGET_FILE:hades> $<TARGET_FILE_BASE_NAME:hades>.hex
    BYPRODUCTS
        hades.hex
    COMMENT
        "Generating HEX image"
    VERBATIM
)

add_custom_command(TARGET hades POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O binary -S $<TARGET_FILE:hades> $<TARGET_FILE_BASE_NAME:hades>.bin
    WORKING_DIRECTORY
        $<TARGET_FILE_DIR:hades>
    BYPRODUCTS
        hades.bin
    COMMENT
        "Generating binary image"
    VERBATIM
)
