//
// Created by Monika on 03.09.2021.
//

#ifndef SR_COMMON_XML_H
#define SR_COMMON_XML_H

#include <Utils/Debug.h>

#include <Utils/FileSystem/Path.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Math/Vector2.h>
#include <Utils/Common/NonCopyable.h>

#include <assimp/contrib/pugixml/src/pugixml.hpp>

namespace SR_UTILS_NS::Xml {
    class Node;

    class Attribute;
    class Document;

    static int32_t g_xml_last_error = 0;

    class SR_DLL_EXPORT Attribute {
    public:
        Attribute()
            : m_attribute()
            , m_valid(false)
        { }

        explicit Attribute(const pugi::xml_attribute &attribute) {
            m_attribute = attribute;
            m_valid = !m_attribute.empty();
        }

    private:
        pugi::xml_attribute m_attribute;
        bool m_valid;

    private:
        SR_NODISCARD bool CheckError(const std::string &msg) const {
            if (m_valid) {
                return true;
            }
            else {
                SRAssert2(false, msg);
                g_xml_last_error = -1;
                return false;
            }
        }

    public:
        explicit operator bool() const {
            return Valid();
        }

        SR_NODISCARD bool Valid() const { return m_valid; }

        SR_NODISCARD std::string ToString() const;
        SR_NODISCARD int32_t ToInt() const;
        SR_NODISCARD uint32_t ToUInt() const;
        SR_NODISCARD int64_t ToInt64() const;
        SR_NODISCARD uint64_t ToUInt64() const;
        SR_NODISCARD float_t ToFloat() const;
        SR_NODISCARD double_t ToDouble() const;
        SR_NODISCARD bool ToBool() const;

        SR_NODISCARD std::string ToString(const std::string &def) const;
        SR_NODISCARD int32_t ToInt(int32_t def) const;
        SR_NODISCARD uint32_t ToUInt(uint32_t def) const;
        SR_NODISCARD int64_t ToInt64(int64_t def) const;
        SR_NODISCARD uint64_t ToUInt64(uint64_t def) const;
        SR_NODISCARD float_t ToFloat(float_t def) const;
        SR_NODISCARD bool ToBool(bool def) const;
    };

    class SR_DLL_EXPORT Node {
        friend class Document;

    public:
        Node();

        explicit Node(pugi::xml_node node) {
            m_node = node;
            m_valid = !m_node.empty();
        }

    public:
        static Node Empty() {
            return Node();
        }

    public:
        explicit operator bool() const {
            return Valid();
        }

    public:
        SR_NODISCARD bool Valid() const {
            return m_valid;
        }

        SR_NODISCARD std::string Name() const {
            if (!m_valid) {
                SRAssert2(false, "Node::Name() : node is not valid!");
                g_xml_last_error = -4;
                return {};
            }

            return m_node.name();
        }

        SR_NODISCARD std::string_view NameView() const {
            if (!m_valid) {
                SRAssert2(false, "Node::Name() : node is not valid!");
                g_xml_last_error = -4;
                return {};
            }

            return m_node.name();
        }

        SR_NODISCARD Document ToDocument() const;
        SR_NODISCARD Attribute GetAttribute(const std::string &name) const {
            if (!m_valid) {
                SRAssert2(false, "Node::GetAttribute() : node is not valid!");
                g_xml_last_error = -4;
                return Attribute();
            }

            return Attribute(m_node.attribute(name.c_str()));
        }

        SR_NODISCARD Attribute TryGetAttribute(const std::string &name) const {
            return m_valid ? Attribute(m_node.attribute(name.c_str())) : Attribute();
        }

        template<typename T> SR_NODISCARD T TryGetAttribute(const T& def) const {
            return m_valid ? GetAttribute<T>() : def;
        }

        SR_NODISCARD bool HasAttribute(const std::string &name) const {
            return m_valid ? !m_node.attribute(name.c_str()).empty() : false;
        }

        SR_NODISCARD std::vector<Node> TryGetNodes() const;
        SR_NODISCARD std::vector<Node> TryGetNodes(const std::string &name) const;
        SR_NODISCARD std::vector<Node> GetNodes(const std::string &name) const;
        SR_NODISCARD std::vector<Node> GetNodes() const;

        template<typename T> Xml::Node NAppendAttribute(const std::string &name, const T &value) {
            if (!m_valid) {
                SRAssert2(false, "Node::NAppendAttribute() : node is not valid!");
                g_xml_last_error = -2;
                return *this;
            }

            auto attrib = m_node.append_attribute(name.c_str());
            if (attrib.empty())
                return *this;

            if constexpr (std::is_same<T, std::string>() || std::is_same<T, SR_UTILS_NS::StringAtom>()) {
                attrib.set_value(value.c_str());
            }
            else  {
                attrib.set_value(value);
            }

            return *this;
        }

        template<typename T, typename U> Xml::Node NAppendAttributeDef(const std::string &name, const T &value, const U& def) {
            if (!m_valid) {
                SRAssert2(false, "Node::NAppendAttributeDef() : node is not valid!");
                g_xml_last_error = -2;
                return *this;
            }

            if (value != def) {
                auto attrib = m_node.append_attribute(name.c_str());
                if (attrib.empty())
                    return *this;

                if constexpr (std::is_same<T, std::string>()) {
                    attrib.set_value(value.c_str());
                } else {
                    attrib.set_value(value);
                }
            }

            return *this;
        }

        template<typename T> T GetAttribute() const {
            if constexpr (std::is_same<T, SR_MATH_NS::FColor>()) {
                SR_MATH_NS::FColor color;

                color.r = GetAttribute("R").ToFloat();
                color.g = GetAttribute("G").ToFloat();
                color.b = GetAttribute("B").ToFloat();
                color.a = GetAttribute("A").ToFloat();

                return color;
            }
            if constexpr (std::is_same<T, SR_MATH_NS::FVector4>()) {
                SR_MATH_NS::FVector4 vector4;

                vector4.x = GetAttribute("X").ToFloat();
                vector4.y = GetAttribute("Y").ToFloat();
                vector4.z = GetAttribute("Z").ToFloat();
                vector4.w = GetAttribute("W").ToFloat();

                return vector4;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FVector2>()) {
                SR_MATH_NS::FVector2 vector2;

                vector2.x = GetAttribute("X").ToFloat();
                vector2.y = GetAttribute("Y").ToFloat();

                return vector2;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::UVector2>()) {
                SR_MATH_NS::UVector2 vector2;

                vector2.x = GetAttribute("X").ToUInt();
                vector2.y = GetAttribute("Y").ToUInt();

                return vector2;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::IVector2>()) {
                SR_MATH_NS::IVector2 vector2;

                vector2.x = GetAttribute("X").ToInt();
                vector2.y = GetAttribute("Y").ToInt();

                return vector2;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FVector3>()) {
                SR_MATH_NS::FVector3 vector3;

                vector3.x = GetAttribute("X").ToFloat();
                vector3.y = GetAttribute("Y").ToFloat();
                vector3.z = GetAttribute("Z").ToFloat();

                return vector3;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::IVector3>()) {
                SR_MATH_NS::IVector3 vector3;

                vector3.x = GetAttribute("X").ToInt();
                vector3.y = GetAttribute("Y").ToInt();
                vector3.z = GetAttribute("Z").ToInt();

                return vector3;
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::UVector3>()) {
                SR_MATH_NS::UVector3 vector3;

                vector3.x = GetAttribute("X").ToUInt();
                vector3.y = GetAttribute("Y").ToUInt();
                vector3.z = GetAttribute("Z").ToUInt();

                return vector3;
            }
            else if constexpr (std::is_same<T, bool>()) {
                return GetAttribute("Bool").ToBool();
            }
            else if constexpr (std::is_same<T, SR_UTILS_NS::Path>()) {
                return GetAttribute("Path").ToString();
            }
            else if constexpr (std::is_same<T, int8_t>()) {
                return GetAttribute("Int8").ToInt();
            }
            else if constexpr (std::is_same<T, int16_t>()) {
                return GetAttribute("Int16").ToInt();
            }
            else if constexpr (std::is_same<T, int32_t>()) {
                return GetAttribute("Int32").ToInt();
            }
            else if constexpr (std::is_same<T, int64_t>()) {
                return GetAttribute("Int64").ToInt64();
            }
            else if constexpr (std::is_same<T, uint8_t>()) {
                return GetAttribute("UInt8").ToUInt();
            }
            else if constexpr (std::is_same<T, uint16_t>()) {
                return GetAttribute("UInt16").ToUInt();
            }
            else if constexpr (std::is_same<T, uint32_t>()) {
                return GetAttribute("UInt32").ToUInt();
            }
            else if constexpr (std::is_same<T, uint64_t>()) {
                return GetAttribute("UInt64").ToUInt64();
            }
            else if constexpr (std::is_same<T, float_t>()) {
                return GetAttribute("Float").ToFloat();
            }
            else if constexpr (std::is_same<T, double_t>()) {
                return GetAttribute("Double").ToDouble();
            }
            else if constexpr (std::is_same<T, std::string>() || std::is_same<T, SR_UTILS_NS::StringAtom>()) {
                return GetAttribute("String").ToString();
            }
            else
                static_assert("Unknown type!");
        }

        template<typename T> bool AppendAttribute(const T &value) {
            if (!m_valid) {
                SRAssert2(false, "Node::AddAttribute() : node is not valid!");
                g_xml_last_error = -2;
                return false;
            }

            bool hasErrors = false;

            if constexpr (std::is_same<T, SR_MATH_NS::FColor>()) {
                hasErrors |= AppendAttribute("R", value.r);
                hasErrors |= AppendAttribute("G", value.g);
                hasErrors |= AppendAttribute("B", value.b);
                hasErrors |= AppendAttribute("A", value.a);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FVector4>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
                hasErrors |= AppendAttribute("Z", value.z);
                hasErrors |= AppendAttribute("W", value.w);
            }
            else if constexpr (std::is_same<T, bool>()) {
                hasErrors |= AppendAttribute("Bool", value);
            }
            else if constexpr (std::is_same<T, float>() || std::is_same<T, float_t>()) {
                hasErrors |= AppendAttribute("Float", value);
            }
            else if constexpr (std::is_same<T, double>() || std::is_same<T, double_t>()) {
                hasErrors |= AppendAttribute("Double", value);
            }
            else if constexpr (std::is_same<T, int8_t>()) {
                hasErrors |= AppendAttribute("Int8", value);
            }
            else if constexpr (std::is_same<T, int16_t>()) {
                hasErrors |= AppendAttribute("Int16", value);
            }
            else if constexpr (std::is_same<T, int32_t>()) {
                hasErrors |= AppendAttribute("Int32", value);
            }
            else if constexpr (std::is_same<T, int64_t>()) {
                hasErrors |= AppendAttribute("Int64", value);
            }
            else if constexpr (std::is_same<T, uint8_t>()) {
                hasErrors |= AppendAttribute("UInt8", value);
            }
            else if constexpr (std::is_same<T, uint16_t>()) {
                hasErrors |= AppendAttribute("UInt16", value);
            }
            else if constexpr (std::is_same<T, uint32_t>()) {
                hasErrors |= AppendAttribute("UInt32", value);
            }
            else if constexpr (std::is_same<T, uint64_t>()) {
                hasErrors |= AppendAttribute("UInt64", value);
            }
            else if constexpr (std::is_same<T, SR_UTILS_NS::Path>()) {
                hasErrors |= AppendAttribute("Path", value);
            }
            else if constexpr (std::is_same<T, std::string>() || std::is_same<T, SR_UTILS_NS::StringAtom>()) {
                hasErrors |= AppendAttribute("String", value);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FVector2>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::UVector2>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::IVector2>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FVector3>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
                hasErrors |= AppendAttribute("Z", value.z);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::IVector3>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
                hasErrors |= AppendAttribute("Z", value.z);
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::UVector3>()) {
                hasErrors |= AppendAttribute("X", value.x);
                hasErrors |= AppendAttribute("Y", value.y);
                hasErrors |= AppendAttribute("Z", value.z);
            }
            else {
                SRHalt("Unknown type!");
                SR_STATIC_ASSERT("Unknown type!");
            }

            return !hasErrors;
        }

        template<typename T> bool AppendAttribute(const std::string &name, const T &value) {
            if (!m_valid) {
                SRAssert2(false, "Node::AddAttribute() : node is not valid!");
                g_xml_last_error = -2;
                return false;
            }

            auto attrib = m_node.append_attribute(name.c_str());
            if (attrib.empty())
                return false;

            if constexpr (std::is_same<T, std::string>() || std::is_same<T, SR_UTILS_NS::StringAtom>()) {
                attrib.set_value(value.c_str());
            }
            else if constexpr (std::is_same<T, SR_UTILS_NS::Path>()) {
                attrib.set_value(value.CStr());
            }
            else {
                attrib.set_value(value);
            }

            return true;
        }

        template<typename T, typename U> bool AppendAttributeDef(const std::string &name, const T &value, const U& def) {
            if (!m_valid) {
                SRAssert2(false, "Node::AddAttribute() : node is not valid!");
                g_xml_last_error = -2;
                return false;
            }

            if (value != def) {
                auto attrib = m_node.append_attribute(name.c_str());
                if (attrib.empty())
                    return false;

                if constexpr (std::is_same<T, std::string>()) {
                    attrib.set_value(value.c_str());
                } else {
                    attrib.set_value(value);
                }
            }

            return true;
        }

        Node AppendChild(const std::string &name);
        Node AppendChild(const Node &node);

        Node AppendNode(const std::string &name) { return AppendChild(name); }
        Node AppendNode(const Node &node) { return AppendChild(node); }

        SR_NODISCARD Node TryGetNode(const std::string &name) const {
            return m_valid ? Node(m_node.child(name.c_str())) : Node();
        }

        SR_NODISCARD Node GetNode(const std::string &name) const {
            if (!m_valid) {
                SRAssert2(false, "Node::GetNode() : node is not valid!");
                g_xml_last_error = -2;
                return Node();
            }

            return Node(m_node.child(name.c_str()));
        }

    private:
        pugi::xml_node m_node;
        bool m_valid;

    };

    class SR_DLL_EXPORT Document : public NonCopyable {
    public:
        Document() {
            m_valid = false;
        }

        Document(Document&& document) noexcept
            : m_document(std::exchange(document.m_document, {}))
            , m_valid(std::exchange(document.m_valid, {}))
            , m_path(std::exchange(document.m_path, {}))
        { }

        ~Document() override {
            if (m_document) {
                delete m_document;
            }
        }

        Document& operator=(Document&& document) noexcept {
            m_document = std::exchange(document.m_document, {});
            m_valid = std::exchange(document.m_valid, {});
            m_path = std::exchange(document.m_path, {});
            return *this;
        }

    private:
        pugi::xml_document* m_document = nullptr;
        bool m_valid;
        std::string m_path;
    public:
        static Document Empty() {
            return Document();
        }

        static Document New() {
            auto xml = Document();
            xml.m_valid = true;
            xml.m_path = "None";
            xml.m_document = new pugi::xml_document();
            return xml;
        }

        static Document Load(const SR_UTILS_NS::Path &path);

        static int32_t GetLastError() {
            auto last = Xml::g_xml_last_error;
            Xml::g_xml_last_error = 0;
            return last;
        }

    public:
        Xml::Node AppendChild(const std::string& name) {
            if (!m_valid) {
                SRAssert2(false,"Document::AppendChild() : document is not valid!");
                g_xml_last_error = -2;
                return Node();
            }

            auto node = m_document->append_child(name.c_str());
            return Node(node);
        }

        bool Save(const SR_UTILS_NS::Path& path) const { /// NOLINT
            if (!path.Exists()) {
                path.Create();
            }

            if (!m_document->save_file(path.CStr())) {
                SR_ERROR("Document::Save() : failed save to file!\n\tPath: " + path.ToString());
                return false;
            }

            return true;
        }

        SR_NODISCARD std::string Dump() const;

        SR_NODISCARD Node Root() const {
            return Node(m_document->root());
        }

        SR_NODISCARD Node TryRoot() const {
            if (!Valid())
                return Node();

            return Node(m_document->root());
        }

        SR_NODISCARD Node DocumentElement() const {
            return Node(m_document->document_element());
        }

        SR_NODISCARD bool Valid() const { return m_valid; }

        operator bool() const { /// NOLINT
            return Valid();
        }
    };

    template<bool NeedConvert> static SR_MATH_NS::FColor NodeToColor(const Xml::Node& node) {
        if constexpr(NeedConvert) {
            return SR_MATH_NS::FColor(
                    node.TryGetAttribute("r").ToFloat(0.f) / 255.f,
                    node.TryGetAttribute("g").ToFloat(0.f) / 255.f,
                    node.TryGetAttribute("b").ToFloat(0.f) / 255.f,
                    node.TryGetAttribute("a").ToFloat(0.f) / 255.f
            );
        }
        else {
            return SR_MATH_NS::FColor(
                    node.TryGetAttribute("r").ToFloat(0.f),
                    node.TryGetAttribute("g").ToFloat(0.f),
                    node.TryGetAttribute("b").ToFloat(0.f),
                    node.TryGetAttribute("a").ToFloat(0.f)
            );
        }
    }

    SR_MAYBE_UNUSED static void AppendColorNode(Xml::Node& node, const SR_MATH_NS::FColor& color) {
        node.AppendChild("Color")
            .NAppendAttribute("r", color.r * 255.f)
            .NAppendAttribute("g", color.g * 255.f)
            .NAppendAttribute("b", color.b * 255.f)
            .NAppendAttribute("a", color.a * 255.f);
    }
}

#endif //SR_COMMON_XML_H
