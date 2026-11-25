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
#include <Utils/Serialization/JsonValue.h>

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

        /// @method
        bool SetState(const JsonValue& state);
        /// @method @evaluate
        bool HasState() const {
            return m_hasState;
        }
        /// @method
        void RemoveState();
        /// @method @evaluate
        JsonValue GetState() const {
            return m_state.Clone();
        }
        /// @method
        void SetMetadata(const JsonValue& metadata) {
            m_metadata = metadata.Clone();
        }
        /// @method @evaluate
        JsonValue GetMetadata() const {
            return m_metadata.Clone();
        }
        /// @method
        bool SetFile(StringView relativePath, StringView bytes);
        /// @method @evaluate
        String GetFile(StringView relativePath) const;
        /// @method @evaluate
        bool HasFile(StringView relativePath) const;
        /// @method
        bool RemoveFile(StringView relativePath);
        /// @method @evaluate
        JsonValue EnumerateFiles() const;
        /// @method
        bool ImportFile(StringView relativePath, const Path& source);
        /// @method
        bool ExportFile(StringView relativePath, const Path& destination);
        /// @method
        void SetReadLimit(uint64_t bytes) {
            m_readLimit = bytes;
        }
        /// @method @evaluate
        String GetError() const {
            return m_error;
        }
        /// @method
        void ClearError() {
            m_error.clear();
        }
        /// @method @evaluate
        bool WasRecovered() const {
            return m_recovered;
        }

    private:
        bool ReadGeneration(const Path& path, StringView generation);
        JsonValue m_state;
        JsonValue m_metadata;
        Map<String, String> m_files;
        uint64_t m_readLimit = 0;
        bool m_hasState = false;
        String m_error;
        bool m_recovered = false;

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
        bool DeleteSlot(StringView group, StringView slotName);
        /// @method
        bool FlushSlot(StringView group, StringView slotName);
        /// @method
        bool SetFaultPoint(StringView point);
        /// @method @evaluate
        String ComputeSha256(StringView text) const;
        /// @method @evaluate
        JsonValue EnumerateSlots(StringView group);
        /// @method
        void SetReadLimit(uint64_t bytes);
        /// @method @evaluate
        String GetError() const {
            return m_error;
        }
        /// @method
        void ClearError() {
            m_error.clear();
        }

    private:
        bool ValidateIds(StringView group, StringView slotName);
        String m_error;
        mutable Vector<SaveSlot::Ptr> m_slots;
        mutable Path m_saveDirectory;
        uint64_t m_readLimit = 0;

    };
}

#endif //SR_COMMON_GAME_SAVE_SYSTEM_H
