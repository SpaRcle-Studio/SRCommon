option(SR_COMMON_UNITY_BUILD "Use unity build instead of incremental." OFF)
option(SR_COMMON_GENERATOR_SOURCES_OUT "${CMAKE_BINARY_DIR}/BuildGenerator/")

set(SR_COMMON_BUILD_GENERATOR_SCRIPT "${PROJECT_SOURCE_DIR}/py")

function(SR_COMMON_GENERATE_BUILD_SOURCES project_dir out_sources_var)
    execute_process(
        COMMAND ${SR_PYTHON_EXECUTABLE} ${SR_COMMON_BUILD_GENERATOR_SCRIPT}
        --project_dir "${project_dir}"
        --out "${SR_COMMON_GENERATOR_SOURCES_OUT}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error_output
    )

    if (NOT result EQUAL "0")
        message(FATAL_ERROR "Build sources generation failed with error:\n${error_output}")
        return()
    endif()

    message(STATUS "${output}")

    string(STRIP "${output}" generated_dir)
    message(STATUS "Build sources generated correctly in:\n\t\t${generated_dir}")

    if (NOT EXISTS "${generated_dir}")
        message(FATAL_ERROR "Generated directory does not exist: ${generated_dir}")
        return()
    else()
        message(STATUS "Generated includes can be found in: ${generated_dir}")
    endif()
endfunction()

