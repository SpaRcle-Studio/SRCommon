## Call the function with the list of files to embed. Things not to forget:
## add_dependencies() : add EmbedResourcesTarget to your project.
## target_include_directories() : include the path to the ${CMAKE_CURRENT_BINARY_DIR}/Utils/include/Utils/EmbedResources.
function(EmbedResources EXPORT_DIRECTORY WORKING_DIRECTORY EMBED_RESOURCES_LIST)
    add_custom_target(EmbedResourcesTarget ALL
            COMMAND python ResourceEmbedder.py --working-directory "${WORKING_DIRECTORY}" --export-directory ${EXPORT_DIRECTORY} --resources "\"${EMBED_RESOURCES_LIST}\""
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Engine/Core/libs/Utils/py")
endfunction()

macro(AddEmbedResource resourcePath)
    set(EMBED_RESOURCES_LIST "${EMBED_RESOURCES_LIST}|${resourcePath}")
endmacro()