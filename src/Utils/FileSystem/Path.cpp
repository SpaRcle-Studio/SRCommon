//
// Created by Monika on 10.12.2021.
//

#include <Utils/FileSystem/Path.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    Path::Path()
        : m_path()
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    { }

    Path::Path(std::string_view path)
        : Path(path.data())
    { }

    Path::Path(std::wstring path)
        : m_path(SR_WS2S(path))
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    Path::Path(SR_UTILS_NS::StringAtom stringAtom)
        : m_path(stringAtom)
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    Path::Path(std::string path)
        : m_path(std::move(path))
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    Path::Path(const char* path)
        : m_path(path)
        , m_name()
        , m_ext()
        , m_hash(SR_UINT64_MAX)
        , m_type(Type::Undefined)
    {
        Update();
        m_hash = SR_HASH_STR(m_path);
    }

    Path Path::Normalize() {
        NormalizeSelf();
        return *this;
    }

    std::string Path::ToString() const {
        return m_path;
    }

    const std::string& Path::ToStringRef() const {
        return m_path;
    }

    std::string_view Path::ToStringView() const {
        return m_path;
    }

    bool Path::IsDir() const {
        return GetType() == Type::Folder;
    }

    bool Path::IsFile() const {
        return GetType() == Type::File;
    }

    std::list<Path> Path::GetFiles() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::File);
    }

    std::list<Path> Path::GetAll() const {
        return SR_PLATFORM_NS::GetAllInDirectory(*this);
    }

    std::list<Path> Path::GetFolders() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::Folder);
    }

    bool Path::Valid() const {
        return m_type != Type::Undefined;
    }

    const char* Path::CStr() const {
        return m_path.c_str();
    }

    void Path::Update() {
        SR_TRACY_ZONE;

        NormalizeSelf();
        ExtractNameAndExt();
    }

    std::string Path::GetExtension() const {
        return std::string(m_ext);
    }

    std::string Path::GetBaseName() const {
        return std::string(m_name);
    }

    Path::Path(const Path& path) {
        m_path = path.m_path;

        ExtractNameAndExt();

        m_hash = path.m_hash;
        m_type = path.m_type;
    }

    size_t Path::GetHash() const {
        return m_hash;
    }

    Path::Type Path::GetType() const {
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

    Path Path::Concat(const Path &path) const {
        if ((!m_path.empty() && m_path.back() != '/') && (!path.IsEmpty() && path.m_path.front() != '/'))
            return m_path + "/" + path.m_path;

        return m_path + path.m_path;
    }

    bool Path::Exists() const {
        return GetType() != Type::Undefined;
    }

    bool Path::Exists(Type type) const {
        if (type == Type::Undefined) {
            return false;
        }
        return GetType() == type;
    }

    void Path::NormalizeSelf() {
        SR_TRACY_ZONE;
        m_path = FileSystem::NormalizePath(m_path);
        m_type = GetType();
    }

    Path Path::ConcatExt(const std::string& ext) const {
        if (ext.empty())
            return *this;

        if (ext[0] == '.')
            return m_path + ext;

        return m_path + "." + ext;
    }

    bool Path::Create() const {
        if (m_path.empty())
            return false;

        if (m_ext.empty()) {
            return FileSystem::CreatePath(m_path);
        }

        return FileSystem::CreatePath(m_path.substr(0, m_path.size() - (m_name.size() + m_ext.size() + 1)));
    }

    bool Path::CreateIfNotExists() const {
        if (!Exists()) {
            return Create();
        }

        return true;
    }

    bool Path::Make(Type type) const {
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

    Path Path::GetPrevious() const {
        if (m_path.empty())
            return m_path;

        if (const auto&& pos = m_path.rfind('/'); pos != std::string::npos) {
            if (pos <= 1)
                return m_path;

            return m_path.substr(0, pos);
        }

        return m_path;
    }

    Path Path::GetFolder() const {
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

    std::string_view Path::GetExtensionView() const {
        return m_ext;
    }

    std::string_view Path::GetBaseNameView() const {
        return m_name;
    }

    uint64_t Path::GetFileHash() const {
        return FileSystem::GetFileHash(m_path);
    }

    uint64_t Path::GetFolderHash(uint64_t deep) const {
        return FileSystem::GetFolderHash(m_path, deep);
    }

    bool Path::IsAbs() const {
        return Platform::IsAbsolutePath(m_path);
    }

    bool Path::IsSubPath(const Path &subPath) const {
        return m_path.find(subPath.m_path) != std::string::npos;
    }

    Path Path::RemoveSubPath(const Path &subPath) const {
        auto&& index = m_path.find(subPath.m_path);

        if (index == std::string::npos) {
            return *this;
        }

        if (m_path.size() == subPath.m_path.size()) {
            return Path();
        }

        return StringUtils::Remove(m_path, index, subPath.m_path.size() + 1);
    }

    Path Path::SelfRemoveSubPath(const Path &subPath) const {
        auto&& index = m_path.find(subPath.m_path);

        if (index == std::string::npos) {
            return std::move(*this);
        }

        if (m_path.size() == subPath.m_path.size()) {
            return Path();
        }

        return StringUtils::Remove(m_path, index, subPath.m_path.size() + 1);
    }

    bool Path::IsHidden() const {
        return Platform::FileIsHidden(m_path);
    }

    std::wstring Path::ToUnicodeString() const {
        return SR_S2WS(m_path);
    }

    std::wstring Path::ToWinApiPath() const {
        auto&& wstring = ToUnicodeString();
        return SR_UTILS_NS::StringUtils::ReplaceAll<std::wstring>(wstring, L"/", L"\\");
    }

    bool Path::IsEmpty() const {
        return m_path.empty();
    }

    SR_NODISCARD bool Path::IsDirEmpty() const {
        return GetAll().empty();
    }

    bool Path::Copy(const Path &destination) const {
        return Platform::Copy(*this, destination);
    }

    std::string Path::GetBaseNameAndExt() const {
        if (m_ext.empty()) {
            return std::string(m_name);
        }
        return std::string(m_name) + "." + std::string(m_ext);
    }

    std::string_view Path::View() const {
        return m_path;
    }

    std::string Path::GetWithoutExtension() const {
        if (m_ext.empty()) {
            return m_path;
        }

        std::string path = m_path;
        path.resize(path.size() - (m_ext.size() + 1));
        return path;
    }

    bool Path::Contains(const std::string &str) const {
        return m_path.find(str) != std::string::npos;
    }

    Path Path::EmplaceFront(const std::string &str) const {
        return str + m_path;
    }

    std::string Path::ConvertToFileName() const {
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

    void Path::ExtractNameAndExt() {
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
}
