## Call the function with the list of files to embed. Things not to forget:
## add_dependencies() : add EmbedResourcesTarget to your project.
## target_include_directories() : include the path to the ${CMAKE_CURRENT_BINARY_DIR}/Utils/include/Utils/EmbedResources.
function(EmbedResources EXPORT_DIRECTORY WORKING_DIRECTORY EMBED_RESOURCES_LIST)
    message("EmbedResources.cmake is going to execute the python script.")
    if (${EXPORT_DIRECTORY} STREQUAL " ")
    if (${WORKING_DIRECTORY} STREQUAL " ")
    if (${EMBED_RESOURCES_LIST} STREQUAL " ")
        add_custom_target(EmbedResourcesTarget
                COMMAND python ResourceEmbedder.py --working-directory "\"\"" --export-directory "\"\"" --resources "\""
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/py")
    endif()
    endif()
    else()
        add_custom_target(EmbedResourcesTarget
                COMMAND python ResourceEmbedder.py --working-directory "${WORKING_DIRECTORY}" --export-directory ${EXPORT_DIRECTORY} --resources "\"${EMBED_RESOURCES_LIST}\""
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Engine/Core/libs/Utils/py")
    endif()
endfunction()

macro(AddEmbedResource resourcePath)
    set(EMBED_RESOURCES_LIST "${EMBED_RESOURCES_LIST}|${resourcePath}")
endmacro()