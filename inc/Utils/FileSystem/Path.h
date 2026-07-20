//
// Created by Monika on 10.12.2021.
//

#ifndef SR_ENGINE_PATH_H
#define SR_ENGINE_PATH_H

#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    class StringAtom;
    class IDeserializer;
    class ISerializer;
    struct SerializationId;

    class SR_COMMON_DLL_API Path {
    public:
        enum class Type {
            Undefined, File, Folder
        };

    public:
        Path();
        Path(const Path& path);
        Path(const char* path);
        Path(SR_UTILS_NS::StringAtom stringAtom);
        Path(const std::string& path);
        Path(const String& path);
        Path(std::string_view path);
        Path(const std::wstring& path);

        Path(Path&& path) noexcept;
        ~Path();

        Path& operator=(Path&& path) noexcept;
        operator const String&(); /** NOLINT */
        Path& operator=(const Path& path);
        Path& operator=(const String& path);
        Path& operator=(const StringView& path);
        Path& operator=(const std::string& path);
        Path& operator=(const char* path);
        Path& operator=(std::string_view path);
        bool operator==(const Path& path) const noexcept;
        char operator[](size_t index) const noexcept;
        bool operator<(const Path& path) const noexcept;
        bool operator>(const Path& path) const noexcept;

    public:
        void Save(ISerializer& serializer, const SerializationId& id) const;
        void Load(IDeserializer& deserializer, const SerializationId& id);

        bool Make(Type type = Type::Undefined) const;
        bool Create() const;
        bool CreateIfNotExists() const;

        void Normalize();

        void clear();
        void Clear();

        SR_NODISCARD bool IsValidPath() const;

        SR_NODISCARD bool Copy(const Path& destination) const;

        SR_NODISCARD String ToString() const;
        SR_NODISCARD std::string ConvertToFileName() const;
        SR_NODISCARD const String& ToStringRef() const;
        SR_NODISCARD std::string_view ToStringView() const;
        SR_NODISCARD std::wstring ToWinApiPath() const;
        SR_NODISCARD std::wstring ToUnicodeString() const;
        SR_NODISCARD size_t GetHash() const;
        SR_NODISCARD uint64_t GetFileHash() const;
        SR_NODISCARD uint64_t GetFolderHash(uint64_t deep = SR_UINT64_MAX) const;
        SR_NODISCARD const char* CStr() const;
        SR_NODISCARD const char* c_str() const;
        SR_NODISCARD uint64_t size() const;
        SR_NODISCARD String& GetInternalUnsafeString();

        SR_NODISCARD Path GetPrevious() const;
        SR_NODISCARD Path GetFolder() const;

        SR_NODISCARD Path Concat(const SR_UTILS_NS::StringAtom path) const;
        SR_NODISCARD Path Concat(const std::string& path) const;
        SR_NODISCARD Path Concat(const std::string_view path) const;
        SR_NODISCARD Path Concat(const char* path) const;
        SR_NODISCARD Path Concat(const Path& path) const;
        SR_NODISCARD Path Concat(const String& path) const;

        SR_NODISCARD Path ConcatExt(const std::string& ext) const;
        SR_NODISCARD Path ConcatExt(const std::string_view& ext) const;
        SR_NODISCARD Path ConcatExt(const char* ext) const;
        SR_NODISCARD Path ConcatExt(SR_UTILS_NS::StringAtom ext) const;

        SR_NODISCARD Path RemoveSubPath(const Path& subPath) const;

        SR_NODISCARD bool empty() const;
        SR_NODISCARD bool IsSubPath(const Path& subPath) const;
        SR_NODISCARD bool Contains(const std::string_view& str) const;
        SR_NODISCARD bool IsHidden() const;
        SR_NODISCARD bool Exists() const;
        SR_NODISCARD bool Exists(Type type) const;

        SR_NODISCARD Type GetType() const;
        SR_NODISCARD bool IsDir() const;
        SR_NODISCARD bool IsFile() const;
        SR_NODISCARD bool IsAbs() const;
        SR_NODISCARD bool IsEmpty() const;

        SR_NODISCARD bool IsDirectoryEmpty() const;

        void GetFiles(Vector<Path>& out) const;
        void GetFolders(Vector<Path>& out) const;
        void GetAll(Vector<Path>& out) const;

        SR_NODISCARD std::string_view GetExtensionView() const;
        SR_NODISCARD std::string_view GetBaseNameView() const;
        SR_NODISCARD std::string_view View() const;
        SR_NODISCARD std::string GetExtension() const;
        SR_NODISCARD std::string GetBaseName() const;
        SR_NODISCARD std::string GetBaseNameAndExt() const;
        SR_NODISCARD std::string_view GetBaseNameAndExtView() const;
        SR_NODISCARD std::string_view GetWithoutExtensionView() const;
        SR_NODISCARD std::string GetWithoutExtension() const;

    private:
        const String& GetNormalized() const;

    private:
        mutable String m_path;
        mutable uint64_t m_hash = SR_UINT64_MAX;
        mutable bool m_isNormalized = false;

    };
}

template<> struct fmt::formatter<SR_UTILS_NS::Path>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(SR_UTILS_NS::Path const& str, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", str.ToStringView());
    }
};

template<> struct std::hash<SR_UTILS_NS::Path> {
    size_t operator()(const SR_UTILS_NS::Path& path) const noexcept {
        return path.GetHash();
    }
};

#endif //SR_ENGINE_PATH_H
