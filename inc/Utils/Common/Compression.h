//
// Created by innerviewer on 2024-04-10.
//
#ifndef SRCOMMON_COMPRESSION_H
#define SRCOMMON_COMPRESSION_H

#include <Utils/FileSystem/Path.h>
//#include <assimp/contrib/zlib/zlib.h>
#include <zlib/zlib.h>

namespace SR_UTILS_NS {
    /*SR_ENUM_NS_CLASS_T(ArchiveFormat, uint8_t,
        z, zip);*/

    class Compression {
    public:
        Compression() = delete;
        ~Compression() = delete;

    public:
        static bool Create(const Path& source, const Path& destination) {

        }
    };
}

#endif //SRCOMMON_COMPRESSION_H
