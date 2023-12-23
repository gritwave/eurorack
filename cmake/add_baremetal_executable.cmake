function(add_baremetal_executable _target _linker_script)
    add_executable(${_target})
    
    set_target_properties(${_target} PROPERTIES
        SUFFIX
            ".elf"
        LINK_DEPENDS
            ${_linker_script}
    )

    target_link_options(${_target} PUBLIC
        -T ${_linker_script}
        -Wl,-Map=${_target}.map,--cref
        -Wl,--check-sections
        -Wl,--unresolved-symbols=report-all
        -Wl,--warn-common
        -Wl,--warn-section-align
        -Wl,--print-memory-usage
    )

    add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND
            ${CMAKE_OBJCOPY}
        ARGS
            -O ihex -S $<TARGET_FILE:${_target}> $<TARGET_FILE_BASE_NAME:${_target}>.hex
        BYPRODUCTS
            ${_target}.hex
        COMMENT
            "Generating HEX image"
        VERBATIM
    )

    add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND
            ${CMAKE_OBJCOPY}
        ARGS
            -O binary -S $<TARGET_FILE:${_target}> $<TARGET_FILE_BASE_NAME:${_target}>.bin
        WORKING_DIRECTORY
            $<TARGET_FILE_DIR:${_target}>
        BYPRODUCTS
            ${_target}.bin
        COMMENT
            "Generating binary image"
        VERBATIM
    )

endfunction()
