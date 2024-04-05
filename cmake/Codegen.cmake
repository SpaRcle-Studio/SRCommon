function(SRUtilsCodegen PROJECT_DIRECTORY CONFIG_FILE EXPORT_DIRECTORY)
    add_custom_target(SRUtilsCodegenTarget
            COMMAND python Codegen.py --project-directory "${PROJECT_DIRECTORY}" --config-file ${CONFIG_FILE} --export-directory "\"${EXPORT_DIRECTORY}\""
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Engine/Core/libs/Utils/py")
endfunction()
