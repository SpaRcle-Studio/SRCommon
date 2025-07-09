macro(SR_COMMON_MAKE_BUILD_DATE_FILE ModuleName)
    string(TIMESTAMP BUILD_DATETIME_STR "%Y-%m-%d %H:%M:%S")

    # путь куда положим файл
    set(BUILD_INFO_HEADER "${CMAKE_BINARY_DIR}/BuildInfo.h")

    # создаём кастомную команду, которая будет перегенерировать файл при каждом билде
    add_custom_command(
            OUTPUT ${BUILD_INFO_HEADER}
            COMMAND ${CMAKE_COMMAND} -E echo "// Auto-generated build info" > ${BUILD_INFO_HEADER}
            COMMAND ${CMAKE_COMMAND} -E echo "#pragma once" >> ${BUILD_INFO_HEADER}
            COMMAND ${CMAKE_COMMAND} -E echo "#define BUILD_DATE_TIME \"${BUILD_DATETIME_STR}\"" >> ${BUILD_INFO_HEADER}
            VERBATIM
            COMMENT "Generating BuildInfo.h"
    )

    # создаём фейковый таргет, который всегда считается устаревшим
    add_custom_target(BuildInfo ALL DEPENDS ${BUILD_INFO_HEADER})

    # связываем с нужной целью (например твоей библиотекой или исполняемым файлом)
    add_dependencies(${ModuleName} BuildInfo)
endmacro()