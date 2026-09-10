//
// Created by Monika on 10.09.2026.
//

#ifndef SR_COMMON_FILE_MODE_H
#define SR_COMMON_FILE_MODE_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_STRUCT_T(FileMode, uint8_t,
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1,
        Map = 1 << 2,
        ReadWrite = Read | Write,
        ReadMap = Read | Map,
        WriteMap = Write | Map,
        ReadWriteMap = Read | Write | Map
    );

    enum class SeekOrigin : uint8_t {
        Begin,
        Current,
        End
    };
}

#endif //SR_COMMON_FILE_MODE_H
