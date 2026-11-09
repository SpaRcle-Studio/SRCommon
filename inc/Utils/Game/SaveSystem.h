//
// Created by Monika on 03.11.2026.
//

#ifndef SR_COMMON_GAME_SAVE_SYSTEM_H
#define SR_COMMON_GAME_SAVE_SYSTEM_H

#include <Utils/Common/Singleton.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Reflection/Value.h>
#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Types/RawPointerHolder.h>

namespace SR_UTILS_NS {
    class SaveSlot : public Serializable, public SR_HTYPES_NS::SharedPtr<SaveSlot> {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<SaveSlot>;

    public:
        SaveSlot();

        void Init(StringView groupName, StringView slotName);
        bool Save(const Path& path);
        bool Load(const Path& path);

        /// @method @evaluate
        SR_NODISCARD StringView GetSlotName() const noexcept { return m_slotName; }
        /// @method @evaluate
        SR_NODISCARD StringView GetGroupName() const noexcept { return m_groupName; }

    private:
        String m_slotName;
        String m_groupName;

        /// @property
        String m_version;
        /// @property
        String m_engineVersion;
        /// @property
        Map<StringAtom, Reflection::Value> m_data;

    };

    /// @noMovable @noCopyable
    class SaveSystem : public Singleton<SaveSystem>, public SRClass {
        SR_REGISTER_SINGLETON(SaveSystem)
        SR_CLASS()
    public:
        /// @method @evaluate
        SR_NODISCARD const Path& GetSaveDirectory() const noexcept;
        /// @method
        SR_NODISCARD SaveSlot::Ptr FindSlot(StringView group, StringView slotName);
        /// @method
        SR_NODISCARD SaveSlot::Ptr OpenSlot(StringView group, StringView slotName);
        /// @method
        void CloseSlot(StringView group, StringView slotName);
        /// @method
        void DeleteSlot(StringView group, StringView slotName);
        /// @method
        void FlushSlot(StringView group, StringView slotName);

    private:
        Vector<SaveSlot::Ptr> m_slots;
        mutable Path m_saveDirectory;

    };
}

#endif //SR_COMMON_GAME_SAVE_SYSTEM_H
