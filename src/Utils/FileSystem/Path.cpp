//
// Created by Monika on 10.12.2021.
//

#include <Utils/FileSystem/Path.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API Path::Path()
        : m_path()
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    { }

    Path::~Path() = default;

    SR_COMMON_DLL_API Path::Path(Path &&path) noexcept
        : m_path(SR_UTILS_NS::Exchange(path.m_path, {}))
        , m_name(std::exchange(path.m_name, {}))
        , m_ext(std::exchange(path.m_ext, {}))
        , m_hash(std::exchange(path.m_hash, {}))
        , m_type(std::exchange(path.m_type, {}))
    { }

    SR_COMMON_DLL_API Path::Path(std::string_view path)
        : Path(std::string(path))
    { }

    SR_COMMON_DLL_API Path::Path(std::wstring path)
        : m_path(SR_WS2S(path))
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    SR_COMMON_DLL_API Path::Path(SR_UTILS_NS::StringAtom stringAtom)
        : m_path(stringAtom)
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    SR_COMMON_DLL_API Path::Path(std::string path)
        : m_path(std::move(path))
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    SR_COMMON_DLL_API Path::Path(const char* path)
        : m_path(path)
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    SR_COMMON_DLL_API Path Path::Normalize() {
        NormalizeSelf();
        return *this;
    }

    SR_COMMON_DLL_API std::string Path::ToString() const {
        return m_path;
    }

    SR_COMMON_DLL_API const std::string& Path::ToStringRef() const {
        return m_path;
    }

    SR_COMMON_DLL_API std::string_view Path::ToStringView() const {
        return m_path;
    }

    SR_COMMON_DLL_API bool Path::IsDir() const {
        return GetType() == Type::Folder;
    }

    SR_COMMON_DLL_API bool Path::IsFile() const {
        return GetType() == Type::File;
    }

    SR_COMMON_DLL_API std::list<Path> Path::GetFiles() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::File);
    }

    SR_COMMON_DLL_API std::list<Path> Path::GetAll() const {
        return SR_PLATFORM_NS::GetAllInDirectory(*this);
    }

    SR_COMMON_DLL_API std::list<Path> Path::GetFolders() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::Folder);
    }

    SR_COMMON_DLL_API bool Path::Valid() const {
        return m_type != Type::Undefined;
    }

    SR_COMMON_DLL_API const char* Path::CStr() const {
        return m_path.c_str();
    }

    SR_COMMON_DLL_API void Path::Update() {
        SR_TRACY_ZONE;

        NormalizeSelf();
        ExtractNameAndExt();
    }

    SR_COMMON_DLL_API std::string Path::GetExtension() const {
        return std::string(m_ext);
    }

    SR_COMMON_DLL_API std::string Path::GetBaseName() const {
        return std::string(m_name);
    }

    SR_COMMON_DLL_API Path::Path(const Path& path) {
        m_path = path.m_path;

        ExtractNameAndExt();

        m_hash = path.m_hash;
        m_type = path.m_type;
    }

    SR_COMMON_DLL_API size_t Path::GetHash() const {
        return m_hash;
    }

    SR_COMMON_DLL_API Path::Type Path::GetType() const {
#ifdef SR_WIN32
    if (m_path.size() < 2 || m_path[1] != ':') {
        return Type::Undefined;
    }
#elif defined(SR_LINUX)
    if (m_path.empty() || m_path[0] != '/') {
        return Type::Undefined;
    }
#endif

        SR_TRACY_ZONE;
#if defined(SR_MSVC) || defined (SR_LINUX)
        struct stat s{};
        if(stat(m_path.c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR) {
                return Type::Folder;
            } else if (s.st_mode & S_IFREG) {
                return Type::File;
            }
        }

        return Type::Undefined;
#elif defined(SR_WIN32)
        DWORD attrib = GetFileAttributes(m_path.c_str());

        if ((attrib & FILE_ATTRIBUTE_DIRECTORY) != 0)
            return Type::Folder;

        return Type::File;
#elif defined(SR_ANDROID)
        /// TODO: будем считать что мы обращаемся только к файлам. Это заглушка - нужно переделать
        return Type::File;
#else
        SRHalt("Unsupported OS!");
        return Type::Undefined;
#endif
    }

    SR_COMMON_DLL_API Path Path::Concat(const Path& path) const {
        if ((!m_path.empty() && m_path.back() != '/') && (!path.IsEmpty() && path.m_path.front() != '/'))
            return m_path + "/" + path.m_path;

        return m_path + path.m_path;
    }

    SR_COMMON_DLL_API bool Path::Exists() const {
        return GetType() != Type::Undefined;
    }

    SR_COMMON_DLL_API bool Path::Exists(Type type) const {
        if (type == Type::Undefined) {
            return false;
        }
        return GetType() == type;
    }

    SR_COMMON_DLL_API void Path::NormalizeSelf() {
        SR_TRACY_ZONE;
        m_path = FileSystem::NormalizePath(m_path);
        m_type = GetType();
    }

    SR_COMMON_DLL_API Path Path::ConcatExt(const std::string& ext) const {
        if (ext.empty())
            return *this;

        if (ext[0] == '.')
            return m_path + ext;

        return m_path + "." + ext;
    }

    SR_COMMON_DLL_API Path Path::ConcatExt(const std::string_view& ext) const {
        return ConcatExt(std::string(ext));
    }

    SR_COMMON_DLL_API Path Path::ConcatExt(const char* ext) const {
        return ConcatExt(std::string(ext));
    }

    SR_COMMON_DLL_API Path Path::ConcatExt(StringAtom ext) const {
        return ConcatExt(ext.ToStringRef());
    }

    SR_COMMON_DLL_API bool Path::Create() const {
        if (m_path.empty())
            return false;

        if (m_ext.empty()) {
            return FileSystem::CreatePath(m_path);
        }

        return FileSystem::CreatePath(m_path.substr(0, m_path.size() - (m_name.size() + m_ext.size() + 1)));
    }

    SR_COMMON_DLL_API bool Path::CreateIfNotExists() const {
        if (!Exists()) {
            return Create();
        }

        return true;
    }

    SR_COMMON_DLL_API void Path::Save(ISerializer& serializer, const SerializationId& id) const {
        serializer.WriteString(m_path, id);
    }

    SR_COMMON_DLL_API void Path::Load(IDeserializer& deserializer, const SerializationId& id) {
        deserializer.ReadString(m_path, id);
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    SR_COMMON_DLL_API bool Path::Make(Type type) const {
        if (m_path.empty())
            return false;

        switch (type) {
            default:
                SRAssert(false);
                SR_FALLTHROUGH;
            case Type::Undefined:
            case Type::File:
                return FileSystem::CreatePath(m_path.substr(0, m_path.size() - (m_name.size() + m_ext.size() + 1)));
            case Type::Folder:
                return FileSystem::CreatePath(m_path);
        }
    }

    SR_COMMON_DLL_API Path Path::GetPrevious() const {
        if (m_path.empty())
            return m_path;

        if (const auto&& pos = m_path.rfind('/'); pos != std::string::npos) {
            if (pos <= 1)
                return m_path;

            return m_path.substr(0, pos);
        }

        return m_path;
    }

    SR_COMMON_DLL_API Path Path::GetFolder() const {
        switch (GetType()) {
            case Type::File:
                return SR_UTILS_NS::StringUtils::GetDirToFileFromFullPath(m_path);
            default:
                SRHalt0();
                SR_FALLTHROUGH;
            case Type::Folder:
            case Type::Undefined:
                return m_path;
        }
    }

    SR_COMMON_DLL_API std::string_view Path::GetExtensionView() const {
        return m_ext;
    }

    SR_COMMON_DLL_API std::string_view Path::GetBaseNameView() const {
        return m_name;
    }

    SR_COMMON_DLL_API uint64_t Path::GetFileHash() const {
        return FileSystem::GetFileHash(m_path);
    }

    SR_COMMON_DLL_API uint64_t Path::GetFolderHash(uint64_t deep) const {
        return FileSystem::GetFolderHash(m_path, deep);
    }

    SR_COMMON_DLL_API bool Path::IsAbs() const {
        return Platform::IsAbsolutePath(m_path);
    }

    SR_COMMON_DLL_API bool Path::IsSubPath(const Path &subPath) const {
        return m_path.find(subPath.m_path) != std::string::npos;
    }

    SR_COMMON_DLL_API Path Path::RemoveSubPath(const Path &subPath) const {
        auto&& index = m_path.find(subPath.m_path);

        if (index == std::string::npos) {
            return *this;
        }

        if (m_path.size() == subPath.m_path.size()) {
            return Path();
        }

        return StringUtils::Remove(m_path, index, subPath.m_path.size() + 1);
    }

    SR_COMMON_DLL_API Path Path::SelfRemoveSubPath(const Path &subPath) const {
        auto&& index = m_path.find(subPath.m_path);

        if (index == std::string::npos) {
            return std::move(*this);
        }

        if (m_path.size() == subPath.m_path.size()) {
            return Path();
        }

        return StringUtils::Remove(m_path, index, subPath.m_path.size() + 1);
    }

    SR_COMMON_DLL_API bool Path::IsHidden() const {
        return Platform::FileIsHidden(m_path);
    }

    SR_COMMON_DLL_API std::wstring Path::ToUnicodeString() const {
        return SR_S2WS(m_path);
    }

    SR_COMMON_DLL_API std::wstring Path::ToWinApiPath() const {
        auto&& wstring = ToUnicodeString();
        return SR_UTILS_NS::StringUtils::ReplaceAll<std::wstring>(wstring, L"/", L"\\");
    }

    SR_COMMON_DLL_API bool Path::IsEmpty() const {
        return m_path.empty();
    }

    SR_NODISCARD SR_COMMON_DLL_API bool Path::IsDirEmpty() const {
        return GetAll().empty();
    }

    SR_COMMON_DLL_API bool Path::Copy(const Path &destination) const {
        return Platform::Copy(*this, destination);
    }

    SR_COMMON_DLL_API std::string Path::GetBaseNameAndExt() const {
        if (m_ext.empty()) {
            return std::string(m_name);
        }
        return std::string(m_name) + "." + std::string(m_ext);
    }

    SR_COMMON_DLL_API std::string_view Path::View() const {
        return m_path;
    }

    SR_COMMON_DLL_API std::string Path::GetWithoutExtension() const {
        if (m_ext.empty()) {
            return m_path;
        }

        std::string path = m_path;
        path.resize(path.size() - (m_ext.size() + 1));
        return path;
    }

    SR_COMMON_DLL_API bool Path::Contains(const std::string &str) const {
        return m_path.find(str) != std::string::npos;
    }

    SR_COMMON_DLL_API Path Path::EmplaceFront(const std::string &str) const {
        return str + m_path;
    }

    SR_COMMON_DLL_API std::string Path::ConvertToFileName() const {
        std::string str = ToString();

        if (str.size() >= 2 && str[1] == ':') {
            str[2] = '-';
        }

        for (auto&& c : str) {
            if (c == '/' || c == '\\') {
                c = '-';
            }
        }

        return str;
    }

    SR_COMMON_DLL_API void Path::ExtractNameAndExt() {
        if (auto&& index = m_path.find_last_of("/\\"); index == std::string::npos) {
            if (index = m_path.find_last_of('.'); index != std::string::npos) {
                m_name = std::string_view { m_path.data(), index };
                m_ext = std::string_view { m_path.data() + index + 1, m_path.size() - index - 1 };
            }
            else {
                m_name = m_path;
                m_ext = std::string_view();
            }
        }
        else {
            ++index;

            if (auto dot = m_path.find_last_of('.'); dot != std::string::npos && dot > index) {
                m_name = std::string_view { m_path.data() + index, dot - index };
                m_ext = std::string_view { m_path.data() + dot + 1, m_path.size() - dot - 1 };
            }
            else {
                m_name = std::string_view { m_path.data() + index, m_path.size() - index };
                m_ext = std::string_view();
            }
        }
    }

    SR_COMMON_DLL_API Path& Path::operator=(Path&& path) noexcept {
        m_path = std::exchange(path.m_path, {});
        m_name = std::exchange(path.m_name, {});
        m_ext = std::exchange(path.m_ext, {});
        m_hash = std::exchange(path.m_hash, {});
        m_type = std::exchange(path.m_type, {});
        return *this;
    }

    SR_COMMON_DLL_API Path& Path::operator=(const Path& path) {
        m_path = path.m_path;

        ExtractNameAndExt();

        m_hash = path.m_hash;
        m_type = path.m_type;

        return *this;
    }

    SR_COMMON_DLL_API const char* Path::c_str() const {
        return CStr();
    }

    SR_COMMON_DLL_API bool Path::empty() const {
        return IsEmpty();
    }

    SR_COMMON_DLL_API Path::operator const std::string&() {
        return m_path;
    }

    SR_COMMON_DLL_API char Path::operator[](size_t index) const noexcept {
        if (index >= m_path.size()) {
            std::cerr << "Path::operator[] : index is out of range!\n";
            SR_MAKE_BREAKPOINT;
            return char();
        }
        return m_path[index];
    }

    SR_COMMON_DLL_API bool Path::operator==(const Path &path) const noexcept {
        return m_path == path.ToStringRef();
    }

    SR_COMMON_DLL_API char& Path::operator[](size_t index) noexcept {
        if (index >= m_path.size()) {
            std::cerr << "Path::operator[] : index is out of range!\n";
            SR_MAKE_BREAKPOINT;
            static char def = char();
            return def;
        }
        return m_path[index];
    }

    SR_COMMON_DLL_API bool Path::operator<(const Path& path) const noexcept {
        return m_path < path.ToStringRef();
    }

    SR_COMMON_DLL_API bool Path::operator>(const Path& path) const noexcept {
        return m_path > path.ToStringRef();
    }
}
