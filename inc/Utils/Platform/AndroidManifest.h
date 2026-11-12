#ifndef SR_UTILS_ANDROID_MANIFEST_H
#define SR_UTILS_ANDROID_MANIFEST_H

#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API bool RewriteAndroidManifestPackage(StringView manifest, StringView package, String& output, String& error);
}

#endif
