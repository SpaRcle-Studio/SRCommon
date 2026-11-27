//
// Created by Monika on 27.11.2026.
//

#ifndef SR_ENGINE_UTILS_RESOURCES_XML_DOCUMENT_H
#define SR_ENGINE_UTILS_RESOURCES_XML_DOCUMENT_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Common/NonCopyable.h>

#ifndef SR_ENGINE_CODEGEN_CLANG_PARSE_MODE

namespace pugi {
    class xml_document;
}

namespace SR_UTILS_NS::Xml {
    class Node;
    class Attribute;

    class SR_COMMON_DLL_API Document : public NonCopyable {
    public:
        Document();
        Document(Document&& document) noexcept;
        ~Document() override;

        Document& operator=(Document&& document) noexcept;

    private:
        pugi::xml_document* m_document = nullptr;
        bool m_valid;
        String m_path;

    public:
        static Document Empty();
        static Document New();
        static Document Load(const SR_UTILS_NS::Path &path);
        static int32_t GetLastError();

    public:
        Xml::Node AppendChild(const String& name);
        bool Save(const SR_UTILS_NS::Path& path) const; /// NOLINT

        SR_NODISCARD String Dump() const;
        SR_NODISCARD Node Root() const;
        SR_NODISCARD Node TryRoot() const;
        SR_NODISCARD Node DocumentElement() const;
        SR_NODISCARD bool Valid() const;

        operator bool() const; /// NOLINT
    };
}

#endif

#endif //SR_ENGINE_UTILS_RESOURCES_XML_DOCUMENT_H
