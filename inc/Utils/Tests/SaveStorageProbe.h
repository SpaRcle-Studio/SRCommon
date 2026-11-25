//
// Created by Codex on 25.11.2026.
//

#ifndef SR_ENGINE_UTILS_TESTS_SAVE_STORAGE_PROBE_H
#define SR_ENGINE_UTILS_TESTS_SAVE_STORAGE_PROBE_H

#include <Utils/Types/String.h>
#include <Utils/TypeTraits/SRClass.h>

namespace SR_UTILS_NS {
    class SaveStorageProbe : public SRClass {
        SR_CLASS()
    public:
        /// @method
        String Run();
        /// @method
        String PrepareRestart();
        /// @method
        String VerifyRestart();
    };
}

#endif // SR_ENGINE_UTILS_TESTS_SAVE_STORAGE_PROBE_H
