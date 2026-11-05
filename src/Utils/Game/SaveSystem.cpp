//
// Created by Monika on 03.11.2026.
//

#include <Utils/Game/SaveSystem.h>
#include <Utils/Platform/Platform.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/DirectoryVFSBackend.h>

#include <Codegen/SaveSystem.generated.hpp>

namespace SR_UTILS_NS {
    SaveSlot::SaveSlot()
        : Serializable()
        , SR_HTYPES_NS::SharedPtr<SaveSlot>(this, SharedPtrPolicy::Automatic)
    { }

    void SaveSlot::Init(StringView groupName, StringView slotName) {
        m_groupName = groupName;
        m_slotName = slotName;
        m_version = SR_PLATFORM_NS::GetApplicationInfo().engineVersion;
        m_engineVersion = SR_PLATFORM_NS::GetApplicationInfo().version;
    }

    bool SaveSlot::Save(const Path& path) {
        if (path.IsFile()) {
            SR_ERROR("SaveSlot::Save() : path is a file! Path: {}", path);
            return false;
        }

        if (!path.CreateDirectories()) {
            SR_ERROR("SaveSlot::Save() : failed to create directories! Path: {}", path);
            return false;
        }

        return true;
    }

    bool SaveSlot::Load(const Path& path) {
        if (!path.IsDir()) {
            SR_ERROR("SaveSlot::Load() : path is not a directory! Path: {}", path);
            return false;
        }

        return true;
    }

    const Path& SaveSystem::GetSaveDirectory() const noexcept {
        if (m_saveDirectory.empty()) {
            m_saveDirectory =
                SR_PLATFORM_NS::GetUserDataDirectory()
                .Concat(SR_PLATFORM_NS::GetApplicationInfo().organization)
                .Concat(SR_PLATFORM_NS::GetApplicationInfo().name);
            m_pVFSBackend = new DirectoryVFSBackend(m_saveDirectory);
            m_pVFSBackend.SetDeleter([](IVFSBackend* pVFS) {
                VFS::Instance().Unmount(pVFS);
                delete pVFS;
            });
            VFS::Instance().Mount(m_saveDirectory, m_pVFSBackend.Get(), 0);
        }
        return m_saveDirectory;
    }

    SaveSlot::Ptr SaveSystem::FindSlot(StringView group, StringView slotName) {
        for (auto&& pSlot : m_slots) {
            if (pSlot->GetGroupName() == group && pSlot->GetSlotName() == slotName) {
                return pSlot;
            }
        }
        return nullptr;
    }

    SaveSlot::Ptr SaveSystem::OpenSlot(StringView group, StringView slotName) {
        if (auto&& pSlot = FindSlot(group, slotName)) {
            return pSlot;
        }

        SaveSlot::Ptr pSlot = new SaveSlot();

        auto&& slotPath = group.empty() ? GetSaveDirectory().Concat(slotName) : GetSaveDirectory().Concat(group).Concat(slotName);
        if (slotPath.IsDir()) {
            SR_INFO("SaveSystem::OpenSlot() : reading existing slot...\n\tPath: {}", slotPath);
            pSlot->Init(group, slotName);
            m_slots.emplace_back(pSlot);
            return pSlot;
        }

        SR_INFO("SaveSystem::OpenSlot() : creating new slot...\n\tPath: {}", slotPath);
        pSlot->Init(group, slotName);
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

    void SaveSystem::DeleteSlot(StringView group, StringView slotName) {
        auto&& slotPath = group.empty() ? GetSaveDirectory().Concat(slotName) : GetSaveDirectory().Concat(group).Concat(slotName);
        SR_INFO("SaveSystem::DeleteSlot() : deleting slot...\n\tPath: {}", slotPath);
        if (slotPath.IsDir()) {
            VFS::Instance().Delete(slotPath);
        }
        CloseSlot(group, slotName);
    }

    void SaveSystem::FlushSlot(StringView group, StringView slotName) {
        if (auto&& pSlot = FindSlot(group, slotName)) {
            auto&& slotPath = group.empty() ? GetSaveDirectory().Concat(slotName) : GetSaveDirectory().Concat(group).Concat(slotName);
            SR_INFO("SaveSystem::FlushSlot() : flushing slot...\n\tPath: {}", slotPath);
            pSlot->Save(slotPath);
        }
    }
}