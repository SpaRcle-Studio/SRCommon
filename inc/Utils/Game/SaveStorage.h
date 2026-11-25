//
// Created by Codex on 25.11.2026.
//

#ifndef SR_ENGINE_UTILS_GAME_SAVE_STORAGE_H
#define SR_ENGINE_UTILS_GAME_SAVE_STORAGE_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Serialization/JsonValue.h>

namespace SR_UTILS_NS::SaveStorage {
    bool ValidId(StringView id);
    bool Read(const Path& path, String& data, uint64_t limit = SR_UINT64_MAX);
    bool Write(const Path& path, StringView data);
    String Digest(StringView data);
    String UtcNow();
    String LocalTime(StringView utc);
    uint64_t GenerationId(StringView name);
    Vector<String> Generations(const Path& path);
    bool HasPublication(const Path& path);
    bool ValidFilePath(StringView path);
    bool ConflictingFilePaths(StringView left, StringView right);
    bool SetFaultPoint(StringView point);
    bool ConsumeFault(StringView point);
    void SetText(JsonValue& object, StringView key, StringView text);
    void SetInteger(JsonValue& object, StringView key, int64_t integer);
}

#endif // SR_ENGINE_UTILS_GAME_SAVE_STORAGE_H
