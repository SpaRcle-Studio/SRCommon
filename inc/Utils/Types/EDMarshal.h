//
// Created by Monika on 29.06.2022.
//

#ifndef SR_ENGINE_EDMARSHAL_H
#define SR_ENGINE_EDMARSHAL_H

#include <Utils/Common/MarshalUtils.h>

#if SR_MARSHAL_ENCODE_AND_DECODE
namespace SR_HTYPES_NS {
    class MarshalDecodeNode;
}

namespace std {
    template<> struct hash<SR_HTYPES_NS::MarshalDecodeNode> {
        size_t operator()(SR_HTYPES_NS::MarshalDecodeNode const& value) const;
    };
}

namespace SR_HTYPES_NS {
    struct MarshalAttribute {
#if SR_MARSHAL_USE_LIST
        std::string m_name;
#endif
        StandardType m_type;
        std::string m_data;
    };

    class SR_DLL_EXPORT MarshalEncodeNode : public SR_UTILS_NS::NonCopyable {
        friend class MarshalDecodeNode;
    public:
        MarshalEncodeNode();
        explicit MarshalEncodeNode(const std::string &name);
        MarshalEncodeNode(MarshalEncodeNode &&marshal) noexcept {
            m_hasNodes = std::exchange(marshal.m_hasNodes, {});
            m_count = std::exchange(marshal.m_count, {});
            m_name = std::exchange(marshal.m_name, {});
#ifdef SR_MINGW
            m_stream.swap(marshal.m_stream);
#else
            m_stream = std::exchange(marshal.m_stream, {});
#endif
        }

        MarshalEncodeNode &operator=(MarshalEncodeNode &&marshal) noexcept {
            m_hasNodes = std::exchange(marshal.m_hasNodes, {});
            m_count = std::exchange(marshal.m_count, {});
            m_name = std::exchange(marshal.m_name, {});
#ifdef SR_MINGW
            m_stream.swap(marshal.m_stream);
#else
            m_stream = std::exchange(marshal.m_stream, {});
#endif
            return *this;
        }

    public:
        static MarshalEncodeNode Load(const Path &path);
        static MarshalEncodeNode LoadFromMemory(const std::string &data);

    public:
        bool Save(const Path &path, MarshalSaveMode mode = MarshalSaveMode::Binary) const;
        SR_NODISCARD std::stringstream Save() const;
        SR_NODISCARD bool Empty() const { return m_count == 0; }
        SR_NODISCARD bool Valid() const { return !Empty() && m_stream.rdbuf()->in_avail() > 0; }
        SR_NODISCARD std::string ToString() const;
        SR_NODISCARD MarshalDecodeNode Decode() const;

        template<typename T, typename U> MarshalEncodeNode &SR_FASTCALL AppendDef(const T &value, const U &def) {
            if constexpr (std::is_same<T, Math::FColor>()) {
                AppendDef("r", value.r, def);
                AppendDef("g", value.g, def);
                AppendDef("b", value.b, def);
                AppendDef("a", value.a, def);
            }
            else if constexpr (std::is_same<T, Math::IVector2>() || std::is_same<T, Math::FVector2>()) {
                AppendDef("x", value.x, def);
                AppendDef("y", value.y, def);
            }
            else if constexpr (std::is_same<T, Math::IVector3>() || std::is_same<T, Math::FVector3>()) {
                AppendDef("x", value.x, def);
                AppendDef("y", value.y, def);
                AppendDef("z", value.z, def);
            }
            else {
                SR_STATIC_ASSERT("unknown type!");
            }

            return *this;
        }

        template<typename T> MarshalEncodeNode &SR_FASTCALL Append(const T &value) {
            if constexpr (std::is_same<T, MarshalEncodeNode>()) {
                m_stream << value.Save().rdbuf(); /// move buffer
                ++m_count;
                m_hasNodes = true;
                return *this;
            }
            else if constexpr (std::is_same<T, Math::FColor>()) {
                Append("r", value.r);
                Append("g", value.g);
                Append("b", value.b);
                Append("a", value.a);
            }
            else if constexpr (std::is_same<T, Math::IVector2>() || std::is_same<T, Math::FVector2>()) {
                Append("x", value.x);
                Append("y", value.y);
            }
            else if constexpr (std::is_same<T, Math::IVector3>() || std::is_same<T, Math::FVector3>()) {
                Append("x", value.x);
                Append("y", value.y);
                Append("z", value.z);
            }
            else {
                SR_STATIC_ASSERT("unknown type!");
            }

            return *this;
        }

        template<typename T, typename U> MarshalEncodeNode &SR_FASTCALL AppendDef(const std::string &name, const T &value, const U &def) {
            if (value == def)
                return *this;

            return Append(name, value);
        }

        template<typename T> MarshalEncodeNode &SR_FASTCALL Append(const std::string &name, const T &value);

    private:
        std::string m_name;
        bool m_hasNodes = false;

        std::stringstream m_stream;
        volatile uint16_t m_count = SR_UINT16_MAX;

    };

    class SR_DLL_EXPORT MarshalDecodeNode {
        friend class MarshalEncodeNode;
    public:
        explicit MarshalDecodeNode(const char *name)
            : m_name(name)
        { }

        explicit MarshalDecodeNode(std::string name)
            : m_name(std::move(name))
        { }

        MarshalDecodeNode &operator=(MarshalDecodeNode &&marshal) noexcept {
            m_name = std::exchange(marshal.m_name, {});
            m_attributes = std::exchange(marshal.m_attributes, {});
            m_nodes = std::exchange(marshal.m_nodes, {});
            return *this;
        }

        MarshalDecodeNode &operator=(const MarshalDecodeNode &marshal) {
            m_name = marshal.m_name;
            m_attributes = marshal.m_attributes;
            m_nodes = marshal.m_nodes;
            return *this;
        }

        MarshalDecodeNode(const MarshalDecodeNode &marshal) {
            m_name = marshal.m_name;
            m_attributes = marshal.m_attributes;
            m_nodes = marshal.m_nodes;
        }

        MarshalDecodeNode(MarshalDecodeNode &&marshal) noexcept {
            m_name = std::exchange(marshal.m_name, {});
            m_attributes = std::exchange(marshal.m_attributes, {});
            m_nodes = std::exchange(marshal.m_nodes, {});
        }

        MarshalDecodeNode() = default;

        explicit operator bool() const {
            return Valid();
        }

    public:
        static MarshalDecodeNode Load(const Path &path);
        static MarshalDecodeNode LoadFromMemory(const std::string &data);

    public:
        SR_NODISCARD bool Empty() const { return m_attributes.empty() && m_nodes.empty(); }
        SR_NODISCARD bool Valid() const { return !Empty(); }
        SR_NODISCARD MarshalEncodeNode Encode() const;

        void Encode(std::stringstream &stream) const;

        MarshalDecodeNode &SR_FASTCALL AppendNode(MarshalDecodeNode &&node);

        template<typename T> MarshalDecodeNode &SR_FASTCALL Append(std::string name, T value) {
            constexpr auto type = SR_UTILS_NS::GetStandardType<T>();

#if SR_MARSHAL_USE_LIST
            if constexpr (Math::IsNumber<T>()) {
                m_attributes.emplace_back(MarshalAttribute{std::move(name), type, std::to_string(value)});
            }
            else {
                m_attributes.emplace_back(MarshalAttribute{std::move(name), type, std::move(value)});
            }
#else
            if constexpr (Math::IsNumber<T>() || Math::IsLogical<T>()) {
                m_attributes[name] = MarshalAttribute{type, std::to_string(value)};
            } else {
                m_attributes[name] = MarshalAttribute{type, std::move(value)};
            }
#endif

            return *this;
        }

        template<typename T> T GetAttribute() const;
        template<typename T, typename U> T SR_FASTCALL GetAttributeDef(const U &def) const;
        template<typename T> T SR_FASTCALL GetAttributeDef(const std::string &name, const T &def) const;
        template<typename T> T SR_FASTCALL GetAttribute(const std::string &name) const;

        SR_NODISCARD MarshalDecodeNode TryGetNodeRef(const std::string &name) const {
            return GetNodeRef(name);
        }

        SR_NODISCARD MarshalDecodeNode SR_FASTCALL TryGetNode(const std::string &name) const;
        SR_NODISCARD const MarshalDecodeNode &GetNodeRef(const std::string &name) const;
        SR_NODISCARD MarshalDecodeNode SR_FASTCALL GetNode(const std::string &name) const;
        SR_NODISCARD std::list<MarshalDecodeNode> GetNodes() const;
        SR_NODISCARD std::string Name() const { return m_name; }
        SR_NODISCARD std::string Dump(uint32_t tab = 0) const;
        SR_NODISCARD std::string ToJson(uint32_t tab = 0, bool root = true) const;

        bool operator==(const MarshalDecodeNode &other) const {
            return m_name == other.m_name;
        }

    private:
        std::string m_name;

#if SR_MARSHAL_USE_LIST
        std::list<MarshalAttribute> m_attributes;
        std::list<MarshalDecodeNode> m_nodes;
#else
        std::unordered_map<std::string, MarshalAttribute> m_attributes;
        std::unordered_map<std::string, std::list<MarshalDecodeNode>> m_nodes;
#endif

    };
}

namespace SR_UTILS_NS {
    namespace MarshalUtils {
        template<typename Stream> SR_HTYPES_NS::MarshalDecodeNode SR_FASTCALL LoadNode(Stream &stream) {
            SR_HTYPES_NS::MarshalDecodeNode node(std::move(MarshalUtils::LoadShortStr<Stream>(stream)));

            const uint16_t count = MarshalUtils::LoadValue<Stream, uint16_t>(stream);

            for (uint16_t i = 0; i < count; ++i) {
                auto &&type = static_cast<StandardType>(MarshalUtils::LoadValue<Stream, uint16_t>(stream));

                if (type == StandardType::Node) {
                    node.AppendNode(std::move(LoadNode<Stream>(stream)));
                }
                else {
                    std::string name = MarshalUtils::LoadShortStr<Stream>(stream);

                    switch (type) {
                        case StandardType::Double: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, double_t>(stream)); break;
                        case StandardType::String: node.Append(std::move(name), MarshalUtils::LoadStr<Stream>(stream)); break;
                        case StandardType::Bool: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, bool>(stream)); break;
                        case StandardType::Int8: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, int8_t>(stream)); break;
                        case StandardType::UInt8: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, uint8_t>(stream)); break;
                        case StandardType::Int16: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, int16_t>(stream)); break;
                        case StandardType::UInt16: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, uint16_t>(stream)); break;
                        case StandardType::Int32: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, int32_t>(stream)); break;
                        case StandardType::UInt32: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, uint32_t>(stream)); break;
                        case StandardType::Int64: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, int64_t>(stream)); break;
                        case StandardType::UInt64: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, uint64_t>(stream)); break;
                        case StandardType::Float: node.Append(std::move(name), MarshalUtils::LoadValue<Stream, float_t>(stream)); break;
                        default: {
                            SRAssert(false);
                            return SR_HTYPES_NS::MarshalDecodeNode();
                        }
                    }
                }
            }
            return node;
        }
    }

    template<typename T> inline T SR_HTYPES_NS::MarshalDecodeNode::GetAttribute() const {
        if constexpr (std::is_same<T, Math::IVector3>()) {
            return Math::IVector3(GetAttribute<int32_t>("x"), GetAttribute<int32_t>("y"), GetAttribute<int32_t>("z"));
        }
        else if constexpr (std::is_same<T, Math::IVector2>()) {
            return Math::IVector2(GetAttribute<int32_t>("x"), GetAttribute<int32_t>("y"));
        }
        else {
            SR_STATIC_ASSERT("unknown type!");
        }
    }

    template<typename T> inline T SR_HTYPES_NS::MarshalDecodeNode::GetAttribute(const std::string &name) const {
#if SR_MARSHAL_USE_LIST
        for (const auto& attribute : m_attributes) {
            if (attribute.m_name == name) {
                if constexpr (Math::IsNumber<T>()) {
                    return LexicalCast<T>(attribute.m_data);
                }
                else
                    return attribute.m_data;
            }
        }

        SRAssert2(false, "Attribute not found! Name: " + name);
        return T();
#else
        auto&& pIt = m_attributes.find(name);

        if (pIt == m_attributes.end()) {
            SRAssert2(false, "Attribute not found! Name: " + name);
            return T();
        }

        if constexpr (Math::IsNumber<T>()) {
            return LexicalCast<T>(pIt->second.m_data);
        }
        else {
            return pIt->second.m_data;
        }
#endif
    }

    template<typename T, typename U> inline T SR_HTYPES_NS::MarshalDecodeNode::GetAttributeDef(const U& def) const {
        if constexpr (std::is_same<T, Math::IVector3>()) {
            return Math::IVector3(
                    GetAttributeDef<int32_t>("x", def),
                    GetAttributeDef<int32_t>("y", def),
                    GetAttributeDef<int32_t>("z", def)
            );
        }
        else if constexpr (std::is_same<T, Math::FVector3>()) {
            return Math::FVector3(
                    GetAttributeDef<Math::Unit>("x", def),
                    GetAttributeDef<Math::Unit>("y", def),
                    GetAttributeDef<Math::Unit>("z", def)
            );
        }
        else if constexpr (std::is_same<T, Math::IVector2>()) {
            return Math::IVector2(GetAttributeDef<int32_t>("x", def), GetAttributeDef<int32_t>("y", def));
        }
        else {
            SR_STATIC_ASSERT("unknown type!");
        }
    }

    template<typename T> inline T SR_HTYPES_NS::MarshalDecodeNode::GetAttributeDef(const std::string& name, const T& def) const {
#if SR_MARSHAL_USE_LIST
        for (const auto& attribute : m_attributes) {
            if (attribute.m_name == name) {
                if constexpr (Math::IsNumber<T>()) {
                    return LexicalCast<T>(attribute.m_data);
                }
                else
                    return attribute.m_data;
            }
        }

        return def;
#else
        if (m_attributes.count(name) == 1)
            return GetAttribute<T>(name);

        return def;
#endif
    }

    template<typename T> inline SR_HTYPES_NS::MarshalEncodeNode &SR_HTYPES_NS::MarshalEncodeNode::Append(const std::string &name, const T &value) {
        if (m_hasNodes) {
            SRAssert2(false, "Attributes can be added before node adding!");
            return *this;
        }

        constexpr auto type = GetStandardType<T>();

        MarshalUtils::SaveValue(m_stream, type);
        MarshalUtils::SaveShortString(m_stream, name);

        if constexpr (Math::IsNumber<T>()) {
            MarshalUtils::SaveValue(m_stream, value);
        }
        else {
            MarshalUtils::SaveString(m_stream, value);
        }

        ++m_count;

        return *this;
    }
}

SR_INLINE size_t std::hash<SR_HTYPES_NS::MarshalDecodeNode>::operator()(const SR_HTYPES_NS::MarshalDecodeNode &value) const {
    std::hash<std::string> h;
    return h(value.Name()) + 0x9e3779b9 + (0 << 6u) + (0 >> 2u);
}
#endif

#endif //SR_ENGINE_EDMARSHAL_H
