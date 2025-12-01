//
// Created by Monika on 10.12.2021.
//

#include <Utils/FileSystem/Path.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    Path::Path() = default;
    Path::~Path() = default;

    Path::Path(Path&& path) noexcept
        : m_path(SR_UTILS_NS::Exchange(path.m_path, {}))
        , m_hash(std::exchange(path.m_hash, {}))
        , m_isNormalized(std::exchange(path.m_isNormalized, {}))
    { }

    Path::Path(std::string_view path)
        : Path(std::string(path))
    { }

    Path::Path(const std::wstring& path)
        : m_path(SR_WS2S(path))
    { }

    Path::Path(SR_UTILS_NS::StringAtom stringAtom)
        : m_path(stringAtom)
    { }

    Path::Path(std::string path)
        : m_path(std::move(path))
    { }

    Path::Path(const Path& path) = default;

    Path::Path(const char* path)
        : m_path(path)
    { }

    Path& Path::operator=(Path&& path) noexcept {
        m_path = std::exchange(path.m_path, {});
        m_hash = std::exchange(path.m_hash, {});
        m_isNormalized = std::exchange(path.m_isNormalized, {});
        return *this;
    }

    Path& Path::operator=(const Path& path) = default;

    std::string Path::ToString() const {
        return GetNormalized();
    }

    const std::string& Path::ToStringRef() const {
        return GetNormalized();
    }

    std::string_view Path::ToStringView() const {
        return GetNormalized();
    }

    bool Path::IsDir() const {
        return GetType() == Type::Folder;
    }

    SR_COMMON_DLL_API bool Path::IsFile() const {
        return GetType() == Type::File;
    }

    SR_COMMON_DLL_API std::list<Path> Path::GetFiles() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::File);
    }

    std::list<Path> Path::GetAll() const {
        return SR_PLATFORM_NS::GetAllInDirectory(*this);
    }

    std::list<Path> Path::GetFolders() const {
        return SR_PLATFORM_NS::GetInDirectory(*this, Path::Type::Folder);
    }

    const char* Path::CStr() const {
        return GetNormalized().c_str();
    }

    std::string Path::GetExtension() const {
        return std::string(GetExtensionView());
    }

    std::string Path::GetBaseName() const {
        return std::string(GetBaseNameView());
    }

    size_t Path::GetHash() const {
        if (m_hash == SR_UINT64_MAX) {
            m_hash = SR_HASH_STR(GetNormalized());
        }
        return m_hash;
    }

    Path::Type Path::GetType() const {
        return SR_PLATFORM_NS::GetPathType(GetNormalized());
    }

    Path Path::Concat(const std::string_view path) const {
        return Concat(std::string(path));
    }

    Path Path::Concat(const SR_UTILS_NS::StringAtom path) const {
        return Concat(path.ToStringRef());
    }

    Path Path::Concat(const char* path) const {
        return Concat(std::string(path));
    }

    Path Path::Concat(const std::string& path) const {
        auto&& normalized = GetNormalized();

        if ((!normalized.empty() && normalized.back() != '/') && (!path.empty() && path.front() != '/')) {
            return normalized + "/" + path;
        }

        return normalized + path;
    }

    Path Path::Concat(const Path& path) const {
        return Concat(path.GetNormalized());
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

    Path Path::ConcatExt(const std::string& ext) const {
        auto&& normalized = GetNormalized();

        if (ext.empty()) {
            return *this;
        }

        if (ext[0] == '.') {
            return normalized + ext;
        }

        return normalized + "." + ext;
    }

    Path Path::ConcatExt(const std::string_view& ext) const {
        return ConcatExt(std::string(ext));
    }

    Path Path::ConcatExt(const char* ext) const {
        return ConcatExt(std::string(ext));
    }

    Path Path::ConcatExt(StringAtom ext) const {
        return ConcatExt(ext.ToStringRef());
    }

    bool Path::Create() const {
        auto&& normalized = GetNormalized();

        if (normalized.empty()) {
            return false;
        }

        std::string_view extension = GetExtensionView();
        if (extension.empty()) {
            return FileSystem::CreatePath(normalized);
        }

        return FileSystem::CreatePath(normalized.substr(0, normalized.size() - (GetBaseNameView().size() + extension.size() + 1)));
    }

    bool Path::CreateIfNotExists() const {
        if (!Exists()) {
            return Create();
        }

        return true;
    }

    void Path::Save(ISerializer& serializer, const SerializationId& id) const {
        serializer.WriteString(GetNormalized(), id);
    }

    void Path::Load(IDeserializer& deserializer, const SerializationId& id) {
        deserializer.ReadString(m_path, id);
        m_isNormalized = false;
    }

    bool Path::Make(Type type) const {
        auto&& normalized = GetNormalized();
        if (normalized.empty()) {
            return false;
        }

        switch (type) {
            default:
                SRAssert(false);
                SR_FALLTHROUGH;
            case Type::Undefined:
            case Type::File:
                return FileSystem::CreatePath(normalized.substr(0, normalized.size() - (GetBaseNameView().size() + GetExtensionView().size() + 1)));
            case Type::Folder:
                return FileSystem::CreatePath(normalized);
        }
    }

    Path Path::GetPrevious() const {
        auto&& normalized = GetNormalized();
        if (normalized.empty()) {
            return normalized;
        }

        if (const auto&& pos = normalized.rfind('/'); pos != std::string::npos) {
            if (pos <= 1) {
                return normalized;
            }

            return normalized.substr(0, pos);
        }

        return normalized;
    }

    Path Path::GetFolder() const {
        switch (GetType()) {
            case Type::File:
                return SR_UTILS_NS::StringUtils::GetDirToFileFromFullPath(GetNormalized());
            default:
                SRHalt0();
                SR_FALLTHROUGH;
            case Type::Folder:
            case Type::Undefined:
                return GetNormalized();
        }
    }

    std::string_view Path::GetExtensionView() const {
        auto&& normalized = GetNormalized();

        SizeType size = normalized.size();

        for (SizeType i = size; i-- > 0;) {
            char c = normalized[i];
            if (c == '/' || c == '\\') {
                break;  // нет расширения
            }
            if (c == '.') {
                // Если точка последняя — расширения нет
                if (i + 1 == size) {
                    return std::string_view();
                }
                return std::string_view(normalized).substr(i + 1);
            }
        }

        return std::string_view();
    }

    std::string_view Path::GetBaseNameView() const {
        auto&& normalized = GetNormalized();

        SizeType size = normalized.size();
        SizeType dotPos = size;
        SizeType slashPos = 0;

        for (SizeType i = size; i-- > 0;) {
            char c = normalized[i];
            if (c == '/' || c == '\\') {
                slashPos = i + 1;
                break;
            }
            else if (c == '.' && dotPos == size) {
                dotPos = i;
            }
        }

        if (dotPos < slashPos) {
            dotPos = size;
        }

        return std::string_view(normalized).substr(slashPos, dotPos - slashPos);
    }

    uint64_t Path::GetFileHash() const {
        return FileSystem::GetFileHash(GetNormalized());
    }

    uint64_t Path::GetFolderHash(uint64_t deep) const {
        return FileSystem::GetFolderHash(GetNormalized(), deep);
    }

    bool Path::IsAbs() const {
        return Platform::IsAbsolutePath(GetNormalized());
    }

    bool Path::IsSubPath(const Path &subPath) const {
        return GetNormalized().find(subPath.GetNormalized()) != std::string::npos;
    }

    Path Path::RemoveSubPath(const Path &subPath) const {
        auto&& normalized = GetNormalized();
        auto&& index = normalized.find(subPath.ToStringRef());

        if (index == std::string::npos) {
            return *this;
        }

        if (normalized.size() == subPath.ToStringRef().size()) {
            return Path();
        }

        return StringUtils::Remove(normalized, index, subPath.ToStringRef().size() + 1);
    }

    bool Path::IsHidden() const {
        return Platform::FileIsHidden(GetNormalized());
    }

    std::wstring Path::ToUnicodeString() const {
        return SR_S2WS(GetNormalized());
    }

    std::wstring Path::ToWinApiPath() const {
        auto&& wstring = ToUnicodeString();
        return SR_UTILS_NS::StringUtils::ReplaceAll<std::wstring>(wstring, L"/", L"\\");
    }

    bool Path::IsEmpty() const {
        return GetNormalized().empty();
    }

    bool Path::IsDirEmpty() const {
        return GetAll().empty();
    }

    bool Path::Copy(const Path &destination) const {
        SR_TRACY_ZONE;
        return Platform::Copy(*this, destination);
    }

     std::string_view Path::GetBaseNameAndExtView() const {
        auto&& normalized = GetNormalized();
        if (const auto pos = normalized.rfind('/'); pos != std::string::npos) {
            return std::string_view(normalized.data() + pos + 1, normalized.size() - pos - 1);
        }
        return normalized;
    }

     std::string Path::GetBaseNameAndExt() const {
        return std::string(GetBaseNameAndExtView());
    }

    std::string_view Path::View() const {
        return GetNormalized();
    }

    std::string_view Path::GetWithoutExtensionView() const {
        auto&& normalized = GetNormalized();
        SR_UTILS_NS::SizeType pos = std::string::npos;
        for (uint32_t i = normalized.size(); i > 0; --i) {
            if (normalized[i] == '/') {
                return normalized;
            }
            if (normalized[i] == '.') {
                pos = i;
                break;
            }
        }

        if (pos == std::string::npos) {
            return normalized;
        }

        return std::string_view(normalized.data(), pos);
    }

    std::string Path::GetWithoutExtension() const {
        return std::string(GetWithoutExtensionView());
    }

    bool Path::Contains(const std::string_view& str) const {
        return GetNormalized().find(str) != std::string::npos;
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

    const char* Path::c_str() const {
        return CStr();
    }

    bool Path::empty() const {
        return IsEmpty();
    }

    Path::operator const std::string&() {
        return GetNormalized();
    }

    char Path::operator[](size_t index) const noexcept {
        if (index >= GetNormalized().size()) {
            SRHalt("Path::operator[] : index is out of range!");
            return char();
        }
        return GetNormalized()[index];
    }

    bool Path::operator==(const Path &path) const noexcept {
        return GetNormalized() == path.ToStringRef();
    }

    bool Path::operator<(const Path& path) const noexcept {
        return GetNormalized() < path.ToStringRef();
    }

    bool Path::operator>(const Path& path) const noexcept {
        return GetNormalized() > path.ToStringRef();
    }

    const std::string& Path::GetNormalized() const {
        if (m_isNormalized) {
            return m_path;
        }
        //m_path = FileSystem::NormalizePath(m_path);
        FileSystem::NormalizePathInPlace(m_path);

        //SR_MAYBE_UNUSED auto&& original = m_path;

        // заменяем все \ на / прямо в строке
        //for (char& c : m_path) {
        //    if (c == '\\') c = '/';
        //}


        m_isNormalized = true;
        return m_path;
    }

    uint64_t Path::size() const {
        return GetNormalized().size();
    }

    bool Path::IsValidPath() const {
        SR_TRACY_ZONE;

        auto&& path = GetNormalized();

        // Пустой путь — сразу невалиден
        if (path.empty()) {
            return false;
        }

        // Проверка каждого символа
        for (unsigned char c : path) {
            if (!FileSystem::IsAllowedPathSymbol(c)) {
                return false;
            }
        }

        // Доп. проверка для Windows: "C:\" или "D:/" и т.п.
        // Если встречается ':', он допустим только сразу после одной буквы
        auto pos = path.find(':');
        if (pos != std::string::npos) {
            if (pos != 1) { return false; }              // только "X:" допустимо
            if (!std::isalpha((unsigned char)path[0])) { // X должен быть буквой
                return false;
            }
        }

        return true;
    }

    void Path::Clear() {
        clear();
    }

    void Path::clear() {
        m_path.clear();
        m_hash = SR_UINT64_MAX;
        m_isNormalized = false;
    }
}
