//
// Created by Monika on 03.11.2026.
//

#include <Utils/Game/SaveSystem.h>
#include <Utils/Game/SaveStorage.h>
#include <Utils/Platform/Platform.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/DirectoryVFSBackend.h>

#include <Codegen/SaveSystem.generated.hpp>

namespace SR_UTILS_NS {
    SR_MAYBE_UNUSED_VAR SaveSystem::Instance();

    SaveSlot::SaveSlot()
        : Serializable()
        , SR_HTYPES_NS::SharedPtr<SaveSlot>(this, SharedPtrPolicy::Automatic)
    {
        m_state.SetNull();
        m_metadata.MakeObject();
    }

    void SaveSlot::Init(StringView groupName, StringView slotName) {
        m_groupName = groupName;
        m_slotName = slotName;
        m_version = SR_PLATFORM_NS::GetApplicationInfo().version;
        m_engineVersion = SR_PLATFORM_NS::GetApplicationInfo().engineVersion;
    }

    bool SaveSlot::SetState(const JsonValue& state) {
        if (!state.View().IsValid()) {
            m_error = "STATE: invalid JSON value";
            return false;
        }
        m_state = state.View().Clone();
        m_hasState = true;
        m_error.clear();
        return true;
    }

    void SaveSlot::RemoveState() {
        m_state.SetNull();
        m_hasState = false;
    }

    bool SaveSlot::SetFile(StringView relativePath, StringView bytes) {
        if (!SaveStorage::ValidFilePath(relativePath)) {
            m_error = "FILE: expected a relative file path without traversal";
            return false;
        }
        for (auto&& entry : m_files) {
            if (entry.first != relativePath && SaveStorage::ConflictingFilePaths(entry.first, relativePath)) {
                m_error = "FILE: conflicting file paths";
                return false;
            }
        }
        m_files[String(relativePath)] = String(bytes);
        m_error.clear();
        return true;
    }

    String SaveSlot::GetFile(StringView relativePath) const {
        const auto found = m_files.find(String(relativePath));
        return found != m_files.end() ? found->second : String();
    }

    bool SaveSlot::HasFile(StringView relativePath) const {
        return m_files.contains(String(relativePath));
    }

    bool SaveSlot::RemoveFile(StringView relativePath) {
        if (!SaveStorage::ValidFilePath(relativePath)) {
            m_error = "FILE: invalid relative file path";
            return false;
        }
        const auto found = m_files.find(String(relativePath));
        if (found == m_files.end()) {
            m_error = "FILE: file is not present in the slot";
            return false;
        }
        m_files.erase(found);
        m_error.clear();
        return true;
    }

    JsonValue SaveSlot::EnumerateFiles() const {
        JsonValue result;
        result.MakeArray();
        for (auto&& entry : m_files) {
            JsonValue file;
            file.MakeObject();
            SaveStorage::SetText(file, "path", entry.first);
            SaveStorage::SetInteger(file, "length", entry.second.size());
            result.Append(file);
        }
        return result;
    }

    bool SaveSlot::ImportFile(StringView relativePath, const Path& source) {
        SR_TRACY_ZONE;
        if (!SaveStorage::ValidFilePath(relativePath)) {
            m_error = "FILE: invalid relative file path";
            return false;
        }
        String bytes;
        if (!SaveStorage::Read(source, bytes, m_readLimit ? m_readLimit : SR_UINT64_MAX)) {
            m_error = "FILE: cannot read source file or read budget exceeded";
            return false;
        }
        return SetFile(relativePath, bytes);
    }

    bool SaveSlot::ExportFile(StringView relativePath, const Path& destination) {
        SR_TRACY_ZONE;
        const auto found = m_files.find(String(relativePath));
        if (found == m_files.end()) {
            m_error = "FILE: file is not present in the slot";
            return false;
        }
        const auto parent = destination.GetPrevious();
        if ((!parent.empty() && !VFS::Instance().CreateDirectories(parent.ToStringView())) || !SaveStorage::Write(destination, found->second)) {
            m_error = "FILE: cannot publish exported file";
            return false;
        }
        m_error.clear();
        return true;
    }

    bool SaveSlot::ReadGeneration(const Path& path, StringView generation, bool metadataOnly) {
        SR_TRACY_ZONE;
        if (!SaveStorage::GenerationId(generation)) {
            m_error = "CURRENT: invalid generation ID";
            return false;
        }
        const auto directory = path.Concat(generation);
        const uint64_t limit = m_readLimit ? m_readLimit : SR_UINT64_MAX;
        String text;
        JsonValue metadata;
        if (!SaveStorage::Read(directory.Concat("metadata.json"), text, limit) || !metadata.Parse(text) ||
            metadata.View().GetKind() != JsonValueKind::Object) {
            m_error = "METADATA: unreadable, malformed or read budget exceeded";
            return false;
        }
        if (!metadata.View().Has("storageVersion")) {
            m_error = "METADATA: missing storage version";
            return false;
        }
        const auto version = metadata.View().Get("storageVersion");
        if (version.GetKind() != JsonValueKind::Integer) {
            m_error = "METADATA: storage version must be an integer";
            return false;
        }
        if (version.GetInt() != 1) {
            m_error = "SCHEMA: unsupported storage version";
            return false;
        }
        if (metadata.View().Get("group").GetString() != m_groupName || metadata.View().Get("slotId").GetString() != m_slotName ||
            metadata.View().Get("generation").GetInt() != static_cast<int64_t>(SaveStorage::GenerationId(generation))) {
            m_error = "METADATA: storage identity does not match";
            return false;
        }

        if (metadataOnly) {
            m_metadata = std::move(metadata);
            m_error.clear();
            return true;
        }

        const auto stateFlag = metadata.View().Get("hasState");
        if (stateFlag.GetKind() != JsonValueKind::Boolean) {
            m_error = "METADATA: missing state presence flag";
            return false;
        }
        const bool hasState = stateFlag.GetBool();
        uint64_t consumed = 0;
        JsonValue candidate;
        if (hasState) {
            if (!SaveStorage::Read(directory.Concat("state.json"), text, limit) ||
                metadata.View().Get("payloadLength").GetKind() != JsonValueKind::Integer ||
                static_cast<int64_t>(text.size()) != metadata.View().Get("payloadLength").GetInt() ||
                SaveStorage::Digest(text) != metadata.View().Get("payloadSha256").GetString()) {
                m_error = "CHECKSUM: state length or SHA-256 mismatch, or read budget exceeded";
                return false;
            }
            consumed = text.size();
            if (!candidate.Parse(text)) {
                m_error = "STATE: malformed JSON value";
                return false;
            }
        }

        Map<String, String> files;
        const auto manifest = metadata.View().Get("files");
        if (manifest.GetKind() != JsonValueKind::Array) {
            m_error = "METADATA: missing file manifest";
            return false;
        }
        for (uint32_t i = 0; i < manifest.GetCount(); ++i) {
            const auto entry = manifest.At(i);
            const String name(entry.Get("path").GetString());
            if (!SaveStorage::ValidFilePath(name)) {
                m_error = "FILE: invalid path in file manifest";
                return false;
            }
            for (auto&& existing : files) {
                if (SaveStorage::ConflictingFilePaths(existing.first, name)) {
                    m_error = "FILE: duplicate or conflicting paths in file manifest";
                    return false;
                }
            }
            String bytes;
            if (!SaveStorage::Read(directory.Concat("files").Concat(name), bytes, limit - consumed) ||
                entry.Get("length").GetKind() != JsonValueKind::Integer ||
                static_cast<int64_t>(bytes.size()) != entry.Get("length").GetInt() ||
                SaveStorage::Digest(bytes) != entry.Get("sha256").GetString()) {
                m_error = "CHECKSUM: file length or SHA-256 mismatch, or read budget exceeded";
                return false;
            }
            consumed += bytes.size();
            files[name] = std::move(bytes);
        }
        m_hasState = hasState;
        m_state = std::move(candidate);
        m_metadata = std::move(metadata);
        m_files = std::move(files);
        m_error.clear();
        return true;
    }

    bool SaveSlot::Save(const Path& path) {
        SR_TRACY_ZONE;
        m_error.clear();
        if (!SaveStorage::ValidId(m_groupName) || !SaveStorage::ValidId(m_slotName) ||
            (m_hasState && !m_state.View().IsValid()) || !m_metadata.View().IsValid() || m_metadata.View().GetKind() != JsonValueKind::Object) {
            m_error = "STATE: invalid slot, JSON value or metadata object";
            return false;
        }
        if (!path.CreateDirectories()) {
            m_error = "WRITE: cannot create slot directory";
            return false;
        }
        SaveSlot previous;
        previous.Init(m_groupName, m_slotName);
        previous.SetReadLimit(m_readLimit);
        String previousGeneration;
        auto generations = SaveStorage::Generations(path);
        if (SaveStorage::HasPublication(path)) {
            if (!previous.Load(path)) {
                m_error = previous.GetError();
                return false;
            }
            previousGeneration = "gen_{}"_format(previous.m_metadata.View().Get("generation").GetInt());
            if (!SaveStorage::Write(path.Concat("published_" + previousGeneration), "1")) {
                m_error = "WRITE: cannot preserve previous publication";
                return false;
            }
        }
        const uint64_t sequence = generations.empty() ? 1 : SaveStorage::GenerationId(generations.front()) + 1;
        if (sequence >= SR_INT64_MAX) {
            m_error = "WRITE: generation sequence exhausted";
            return false;
        }
        const String generation = "gen_{}"_format(sequence);
        const auto directory = path.Concat(generation);
        const String payload = m_hasState ? m_state.View().ToJson() : String();
        JsonValue metadata = m_metadata.View().Clone();
        SaveStorage::SetInteger(metadata, "storageVersion", 1);
        SaveStorage::SetText(metadata, "slotId", m_slotName);
        SaveStorage::SetText(metadata, "group", m_groupName);
        SaveStorage::SetInteger(metadata, "generation", sequence);
        SaveStorage::SetText(metadata, "createdUtc", SaveStorage::UtcNow());
        SaveStorage::SetText(metadata, "payloadSha256", SaveStorage::Digest(payload));
        SaveStorage::SetInteger(metadata, "payloadLength", payload.size());
        JsonValue stateFlag;
        stateFlag.SetBool(m_hasState);
        metadata.Set("hasState", stateFlag);

        if (!directory.CreateDirectories() ||
            (m_hasState && !SaveStorage::Write(directory.Concat("state.json"), payload)) ||
            SaveStorage::ConsumeFault("after_state")) {
            m_error = "WRITE: failed to flush state; previous save preserved";
            return false;
        }

        JsonValue manifest;
        manifest.MakeArray();
        for (auto&& entry : m_files) {
            if (!SaveStorage::ValidFilePath(entry.first)) {
                m_error = "FILE: invalid relative file path";
                return false;
            }
            const auto destination = directory.Concat("files").Concat(entry.first);
            if (!VFS::Instance().CreateDirectories(destination.GetPrevious().ToStringView()) || !SaveStorage::Write(destination, entry.second)) {
                m_error = "WRITE: failed to flush user file; previous save preserved";
                return false;
            }
            JsonValue info;
            info.MakeObject();
            SaveStorage::SetText(info, "path", entry.first);
            SaveStorage::SetInteger(info, "length", entry.second.size());
            SaveStorage::SetText(info, "sha256", SaveStorage::Digest(entry.second));
            manifest.Append(info);
        }
        metadata.Set("files", manifest);
        if (SaveStorage::ConsumeFault("after_files") ||
            !SaveStorage::Write(directory.Concat("metadata.json"), metadata.View().ToJson()) ||
            SaveStorage::ConsumeFault("after_metadata")) {
            m_error = "WRITE: failed to flush generation; previous save preserved";
            return false;
        }
        SaveSlot verified;
        verified.Init(m_groupName, m_slotName);
        verified.SetReadLimit(m_readLimit);
        if (!verified.ReadGeneration(path, generation)) {
            m_error = verified.GetError();
            return false;
        }
        if (SaveStorage::ConsumeFault("before_marker") || !SaveStorage::Write(path.Concat("current"), generation)) {
            m_error = "PUBLISH: marker replacement failed; previous save preserved";
            return false;
        }
        SaveStorage::SetText(metadata, "displayTime", SaveStorage::LocalTime(metadata.View().Get("createdUtc").GetString()));
        m_metadata = std::move(metadata);
        m_recovered = false;
        if (SaveStorage::ConsumeFault("after_marker")) {
            SR_WARN("SaveSlot::Save() : injected interruption after marker replacement; generation committed");
            return true;
        }
        if (!SaveStorage::Write(path.Concat("published_" + generation), "1")) {
            SR_WARN("SaveSlot::Save() : publication receipt failed; current marker remains authoritative");
        }
        for (auto&& oldGeneration : generations) {
            if (oldGeneration != previousGeneration) {
                VFS::Instance().Delete(path.Concat(oldGeneration).ToStringView());
                VFS::Instance().Delete(path.Concat("published_" + oldGeneration).ToStringView());
            }
        }
        return true;
    }

    bool SaveSlot::Load(const Path& path) {
        return LoadInternal(path, false);
    }

    bool SaveSlot::LoadMetadata(const Path& path) {
        return LoadInternal(path, true);
    }

    bool SaveSlot::LoadInternal(const Path& path, bool metadataOnly) {
        SR_TRACY_ZONE;
        m_error.clear();
        m_recovered = false;
        String current;
        if (SaveStorage::Read(path.Concat("current"), current, 64) && ReadGeneration(path, current, metadataOnly)) {
            return true;
        }
        if (m_error.starts_with("SCHEMA:")) {
            return false;
        }
        for (auto&& generation : SaveStorage::Generations(path)) {
            String receipt;
            if (generation != current && SaveStorage::Read(path.Concat("published_" + generation), receipt, 8) &&
                receipt == "1" && ReadGeneration(path, generation, metadataOnly)) {
                m_recovered = true;
                SR_WARN("SaveSlot::Load() : recovered {}/{} from {}", m_groupName, m_slotName, generation);
                return true;
            }
            if (m_error.starts_with("SCHEMA:")) {
                return false;
            }
        }
        if (m_error.empty()) {
            m_error = "READ: no published valid generation";
        }
        return false;
    }

    bool SaveSystem::SetFaultPoint(StringView point) {
        return SaveStorage::SetFaultPoint(point);
    }

    String SaveSystem::ComputeSha256(StringView text) const {
        return SaveStorage::Digest(text);
    }

    bool SaveSystem::ValidateIds(StringView group, StringView slotName) {
        m_error.clear();
        const auto& application = SR_PLATFORM_NS::GetApplicationInfo();
        if (application.organization.empty() || application.name.empty() || !GetSaveDirectory().IsAbs()) {
            m_error = "PATH: organization, application name and absolute user data directory are required";
            return false;
        }
        if (!SaveStorage::ValidId(group) || !SaveStorage::ValidId(slotName)) {
            m_error = "ID: only letters, digits, underscore and hyphen are allowed";
            return false;
        }
        return true;
    }

    const Path& SaveSystem::GetSaveDirectory() const noexcept {
        Path directory = SR_PLATFORM_NS::GetApplicationUserDataDirectory().Concat("Save");
        const auto& project = SR_PLATFORM_NS::GetApplicationInfo().project;
        if (!project.empty()) {
            directory = directory.Concat(project);
        }
        if (m_saveDirectory != directory) {
            m_slots.clear();
            m_saveDirectory = directory;
        }
        if (!VFS::Instance().HaveMount(m_saveDirectory.ToStringView())) {
            VFS::Instance().Mount(m_saveDirectory.ToStringView(), new DirectoryVFSBackend(m_saveDirectory.ToStringView()), 0);
        }
        return m_saveDirectory;
    }

    SaveSlot::Ptr SaveSystem::FindSlot(StringView group, StringView slotName) {
        (void)GetSaveDirectory();
        for (auto&& pSlot : m_slots) {
            if (pSlot->GetGroupName() == group && pSlot->GetSlotName() == slotName) {
                return pSlot;
            }
        }
        return nullptr;
    }

    SaveSlot::Ptr SaveSystem::OpenSlot(StringView group, StringView slotName) {
        if (!ValidateIds(group, slotName)) {
            return nullptr;
        }
        if (auto&& pSlot = FindSlot(group, slotName)) {
            return pSlot;
        }

        SaveSlot::Ptr pSlot = new SaveSlot();

        auto&& slotPath = GetSaveDirectory().Concat(group).Concat(slotName);
        pSlot->Init(group, slotName);
        pSlot->SetReadLimit(m_readLimit);
        if (SaveStorage::HasPublication(slotPath)) {
            if (!pSlot->Load(slotPath)) {
                m_error = pSlot->GetError();
                return nullptr;
            }
        }

        SR_INFO("SaveSystem::OpenSlot() : creating new slot...\n\tPath: {}", slotPath);
        m_slots.emplace_back(pSlot);
        return pSlot;
    }

    void SaveSystem::CloseSlot(StringView group, StringView slotName) {
        for (auto it = m_slots.begin(); it != m_slots.end(); ++it) {
            auto&& pSlot = *it;
            if (pSlot->GetGroupName() == group && pSlot->GetSlotName() == slotName) {
                m_slots.erase(it);
                return;
            }
        }
    }

    bool SaveSystem::DeleteSlot(StringView group, StringView slotName) {
        if (!ValidateIds(group, slotName)) {
            return false;
        }
        auto&& slotPath = GetSaveDirectory().Concat(group).Concat(slotName);
        SR_INFO("SaveSystem::DeleteSlot() : deleting slot...\n\tPath: {}", slotPath);
        if (slotPath.IsDir()) {
            VFS::Instance().Delete(slotPath);
        }
        CloseSlot(group, slotName);
        if (slotPath.IsExists()) {
            m_error = "DELETE: slot directory still exists";
            return false;
        }
        return true;
    }

    bool SaveSystem::FlushSlot(StringView group, StringView slotName) {
        if (!ValidateIds(group, slotName)) {
            return false;
        }
        if (auto&& pSlot = FindSlot(group, slotName)) {
            auto&& slotPath = group.empty() ? GetSaveDirectory().Concat(slotName) : GetSaveDirectory().Concat(group).Concat(slotName);
            SR_INFO("SaveSystem::FlushSlot() : flushing slot...\n\tPath: {}", slotPath);
            if (pSlot->Save(slotPath)) {
                return true;
            }
            m_error = pSlot->GetError();
            return false;
        }
        m_error = "SLOT: slot is not open";
        return false;
    }

    void SaveSystem::SetReadLimit(uint64_t bytes) {
        m_readLimit = bytes;
        for (auto&& pSlot : m_slots) {
            pSlot->SetReadLimit(bytes);
        }
    }

    JsonValue SaveSystem::EnumerateSlotPreviews(StringView group) {
        SR_TRACY_ZONE;
        JsonValue result;
        result.MakeArray();
        if (!ValidateIds(group, "enumerate")) {
            return result;
        }
        VFS::Instance().Enumerate(GetSaveDirectory().Concat(group).ToStringView(), [&](const VFSEntry& entry) {
            if (entry.type != FSItemType::Folder || !SaveStorage::ValidId(entry.name)) {
                return;
            }
            SaveSlot slot;
            slot.Init(group, entry.name);
            slot.SetReadLimit(m_readLimit && m_readLimit < 1024 * 1024 ? m_readLimit : 1024 * 1024);
            const auto path = GetSaveDirectory().Concat(group).Concat(entry.name);
            if (slot.LoadMetadata(path)) {
                JsonValue metadata = slot.GetMetadata().Clone();
                SaveStorage::SetText(metadata, "displayTime", SaveStorage::LocalTime(metadata.View().Get("createdUtc").GetString()));
                JsonValue recovered;
                recovered.SetBool(slot.WasRecovered());
                metadata.Set("recovered", recovered);
                result.Append(metadata);
            }
            else if (SaveStorage::HasPublication(path)) {
                m_error = slot.GetError();
            }
        }, false);
        return result;
    }

    String SaveSystem::ReadSlotPreviewFile(StringView group, StringView slotName, StringView relativePath) {
        SR_TRACY_ZONE;
        if (!ValidateIds(group, slotName) || !SaveStorage::ValidFilePath(relativePath)) {
            m_error = "FILE: invalid preview identity or relative path";
            return String();
        }
        SaveSlot slot;
        slot.Init(group, slotName);
        slot.SetReadLimit(m_readLimit && m_readLimit < 1024 * 1024 ? m_readLimit : 1024 * 1024);
        const auto path = GetSaveDirectory().Concat(group).Concat(slotName);
        if (!slot.LoadMetadata(path)) {
            m_error = slot.GetError();
            return String();
        }
        const auto metadata = slot.GetMetadata();
        if (!metadata.Has("files") || metadata.Get("files").GetKind() != JsonValueKind::Array) {
            m_error = "METADATA: missing preview file manifest";
            return String();
        }
        const auto manifest = metadata.Get("files");
        const uint64_t limit = m_readLimit && m_readLimit < 4 * 1024 * 1024 ? m_readLimit : 4 * 1024 * 1024;
        for (uint32_t i = 0; i < manifest.GetCount(); ++i) {
            const auto entry = manifest.At(i);
            if (!entry.Has("path") || entry.Get("path").GetString() != relativePath) {
                continue;
            }
            String bytes;
            const String generation = "gen_{}"_format(metadata.Get("generation").GetInt());
            if (entry.Has("length") && entry.Has("sha256") &&
                entry.Get("length").GetKind() == JsonValueKind::Integer &&
                SaveStorage::Read(path.Concat(generation).Concat("files").Concat(relativePath), bytes, limit) &&
                static_cast<int64_t>(bytes.size()) == entry.Get("length").GetInt() &&
                SaveStorage::Digest(bytes) == entry.Get("sha256").GetString()) {
                return bytes;
            }
            m_error = "CHECKSUM: preview file length or SHA-256 mismatch, or read budget exceeded";
            return String();
        }
        m_error = "FILE: preview file is not present in the slot";
        return String();
    }

    JsonValue SaveSystem::EnumerateSlots(StringView group) {
        SR_TRACY_ZONE;
        JsonValue result;
        result.MakeArray();
        if (!ValidateIds(group, "enumerate")) {
            return result;
        }
        VFS::Instance().Enumerate(GetSaveDirectory().Concat(group).ToStringView(), [&](const VFSEntry& entry) {
            if (entry.type != FSItemType::Folder || !SaveStorage::ValidId(entry.name)) {
                return;
            }
            SaveSlot slot;
            slot.Init(group, entry.name);
            slot.SetReadLimit(m_readLimit);
            if (slot.Load(GetSaveDirectory().Concat(group).Concat(entry.name))) {
                JsonValue metadata = slot.GetMetadata().Clone();
                SaveStorage::SetText(metadata, "displayTime", SaveStorage::LocalTime(metadata.View().Get("createdUtc").GetString()));
                JsonValue recovered;
                recovered.SetBool(slot.WasRecovered());
                metadata.Set("recovered", recovered);
                result.Append(metadata);
            }
            else if (SaveStorage::HasPublication(GetSaveDirectory().Concat(group).Concat(entry.name))) {
                m_error = slot.GetError();
            }
        }, false);
        return result;
    }
}
