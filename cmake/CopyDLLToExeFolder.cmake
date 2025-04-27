function(SR_COPY_SHARED_MODULE_TO_EXECUTABLE_FOLDER TARGET)
    if (TARGET ${TARGET} AND DEFINED SR_EXECUTABLE_DIR)
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if (TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET ${TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${TARGET}>
                ${SR_EXECUTABLE_DIR}/$<TARGET_FILE_NAME:${TARGET}>
                COMMENT "SpaRcle: Copying ${TARGET} to executable folder ${SR_EXECUTABLE_DIR}"
            )
        endif()
    endif()
endfunction()