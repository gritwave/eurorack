project(kyma)

add_executable(kyma)

target_sources(kyma
    PRIVATE
        main.cpp
)

target_link_libraries(kyma
    PRIVATE
        daisy
        gw::audio
)

set_target_properties(kyma PROPERTIES
    SUFFIX
        ".elf"
    LINK_DEPENDS
        ${LINKER_SCRIPT}
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

add_custom_command(TARGET kyma POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
    ARGS
        -O ihex -S $<TARGET_FILE:kyma> $<TARGET_FILE_BASE_NAME:kyma>.hex
    BYPRODUCTS
        kyma.hex
    COMMENT
        "Generating HEX image"
    VERBATIM
)

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
