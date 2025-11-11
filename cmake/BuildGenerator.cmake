option(SR_COMMON_UNITY_BUILD "Use unity build instead of incremental." OFF)
set(SR_COMMON_GENERATOR_SOURCES_OUT "${CMAKE_BINARY_DIR}/BuildGenerator")

function(SR_COMMON_GENERATE_BUILD_SOURCES project_dir)
    execute_process(
        COMMAND ${SR_PYTHON_EXECUTABLE} ${SR_COMMON_BUILD_SCRIPT_PATH}
        --project-path "${project_dir}"
        --out "${SR_COMMON_GENERATOR_SOURCES_OUT}"
        --unity "${SR_COMMON_UNITY_BUILD}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error_output
    )

    if (NOT result EQUAL "0")
        message(FATAL_ERROR "Build sources generation failed with error:\n${error_output}")
        return()
    endif()

    message(STATUS "${output}")
endfunction()

function(SR_COMMON_GET_SOURCES module_name)
    set(src_file "${SR_COMMON_GENERATOR_SOURCES_OUT}/${module_name}/main.srinc")

    if(NOT EXISTS "${src_file}")
        message(FATAL_ERROR "Source include file not found: ${src_file}")
    endif()

    # Read all lines from the file
    file(STRINGS "${src_file}" sources)

    # Return the list to the caller
    set(${module_name}_SOURCES ${sources} PARENT_SCOPE)
endfunction()
