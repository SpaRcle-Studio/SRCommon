//
// Created by Monika on 11.12.2025.
//

#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/Serialization/ObjectDataAccessor.h>
#include <Utils/Types/UnicodeString.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Math/Rect.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Math/AABB.h>
#include <Utils/Serialization/SerializationTraits.h>
#include <Utils/Serialization/SerializationSaveUtils.h>
#include <Utils/Serialization/SerializationLoadUtils.h>

namespace SR_UTILS_NS {
    #include <Utils/Serialization/ObjectDataAccessors.inl.h>

    void ObjectDataAccessor<std::string>::Save(ISerializer& serializer, const std::string& value, const SerializationId& id) {
       serializer.WriteString(value, id);
    }

    void ObjectDataAccessor<std::string>::Load(IDeserializer& deserializer, std::string& value, const SerializationId& id) {
       deserializer.ReadString(value, id);
    }

    void ObjectDataAccessor<SR_UTILS_NS::StringAtom>::Save(ISerializer& serializer, const SR_UTILS_NS::StringAtom& value, const SerializationId& id) {
       serializer.WriteString(value.ToStringView(), id);
    }

    void ObjectDataAccessor<SR_UTILS_NS::StringAtom>::Load(IDeserializer& deserializer, SR_UTILS_NS::StringAtom& value, const SerializationId& id) {
        deserializer.ReadString(value, id);
    }

    void ObjectDataAccessor<SR_HTYPES_NS::UnicodeString>::Save(ISerializer& serializer, const SR_HTYPES_NS::UnicodeString& value, const SerializationId& id) {
        serializer.WriteString(value, id);
    }

    void ObjectDataAccessor<SR_HTYPES_NS::UnicodeString>::Load(IDeserializer& deserializer, SR_HTYPES_NS::UnicodeString& value, const SerializationId& id) {
        deserializer.ReadString(value, id);
    }

    void ObjectDataAccessor<bool>::Save(ISerializer& serializer, const bool value, const SerializationId& id) {
        serializer.WriteBool(value, id);
    }

    void ObjectDataAccessor<bool>::Load(IDeserializer& deserializer, bool& value, const SerializationId& id) {
        deserializer.ReadBool(value, id);
    }

    void ObjectDataAccessor<float_t>::Save(ISerializer& serializer, const float_t value, const SerializationId& id) {
        serializer.WriteFloat(value, id);
    }

    void ObjectDataAccessor<float_t>::Load(IDeserializer& deserializer, float_t& value, const SerializationId& id) {
        deserializer.ReadFloat(value, id);
    }

    void ObjectDataAccessor<double_t>::Save(ISerializer& serializer, const double_t value, const SerializationId& id) {
        serializer.WriteDouble(value, id);
    }

    void ObjectDataAccessor<double_t>::Load(IDeserializer& deserializer, double& value, const SerializationId& id) {
        deserializer.ReadDouble(value, id);
    }

    void ObjectDataAccessor<std::int8_t>::Save(ISerializer& serializer, const std::int8_t value, const SerializationId& id) {
        serializer.WriteInt(value, id);
    }

    void ObjectDataAccessor<std::int8_t>::Load(IDeserializer& deserializer, std::int8_t& value, const SerializationId& id) {
        deserializer.ReadInt(value, id);
    }

    void ObjectDataAccessor<std::int16_t>::Save(ISerializer& serializer, const std::int16_t value, const SerializationId& id) {
        serializer.WriteInt(value, id);
    }

    void ObjectDataAccessor<std::int16_t>::Load(IDeserializer& deserializer, std::int16_t& value, const SerializationId& id) {
        deserializer.ReadInt(value, id);
    }

    void ObjectDataAccessor<std::int32_t>::Save(ISerializer& serializer, const std::int32_t value, const SerializationId& id) {
        serializer.WriteInt(value, id);
    }

    void ObjectDataAccessor<std::int32_t>::Load(IDeserializer& deserializer, std::int32_t& value, const SerializationId& id) {
        deserializer.ReadInt(value, id);
    }

    void ObjectDataAccessor<std::int64_t>::Save(ISerializer& serializer, const std::int64_t value, const SerializationId& id) {
        serializer.WriteInt(value, id);
    }

    void ObjectDataAccessor<std::int64_t>::Load(IDeserializer& deserializer, std::int64_t& value, const SerializationId& id) {
        deserializer.ReadInt(value, id);
    }

    void ObjectDataAccessor<std::uint8_t>::Save(ISerializer& serializer, std::uint8_t value, const SerializationId& id) {
        serializer.WriteUInt(value, id);
    }

    void ObjectDataAccessor<std::uint8_t>::Load(IDeserializer& deserializer, std::uint8_t& value, const SerializationId& id) {
        deserializer.ReadUInt(value, id);
    }

    void ObjectDataAccessor<std::uint16_t>::Save(ISerializer& serializer, std::uint16_t value, const SerializationId& id) {
        serializer.WriteUInt(value, id);
    }

    void ObjectDataAccessor<std::uint16_t>::Load(IDeserializer& deserializer, std::uint16_t& value, const SerializationId& id) {
        deserializer.ReadUInt(value, id);
    }

    void ObjectDataAccessor<std::uint32_t>::Save(ISerializer& serializer, std::uint32_t value, const SerializationId& id) {
        serializer.WriteUInt(value, id);
    }

    void ObjectDataAccessor<std::uint32_t>::Load(IDeserializer& deserializer, std::uint32_t& value, const SerializationId& id) {
        deserializer.ReadUInt(value, id);
    }

    void ObjectDataAccessor<std::uint64_t>::Save(ISerializer& serializer, std::uint64_t value, const SerializationId& id) {
        serializer.WriteUInt(value, id);
    }

    void ObjectDataAccessor<std::uint64_t>::Load(IDeserializer& deserializer, std::uint64_t& value, const SerializationId& id) {
        deserializer.ReadUInt(value, id);
    }

    void ObjectDataAccessor<SR_MATH_NS::Quaternion>::Save(ISerializer& serializer, const SR_MATH_NS::Quaternion& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));
        Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));
        Serialization::SaveCheckDefault(serializer, value.z, SerializationId::Create("z"));
        Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::Quaternion>::Load(IDeserializer& deserializer, SR_MATH_NS::Quaternion& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.x, SerializationId::Create("x"));
        Serialization::Load(deserializer, value.y, SerializationId::Create("y"));
        Serialization::Load(deserializer, value.z, SerializationId::Create("z"));
        Serialization::Load(deserializer, value.w, SerializationId::Create("w"));
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::AABB>::Save(ISerializer& serializer, const SR_MATH_NS::AABB& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.min, SerializationId::Create("min"));
        Serialization::SaveCheckDefault(serializer, value.max, SerializationId::Create("max"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::AABB>::Load(IDeserializer& deserializer, SR_MATH_NS::AABB& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.min, SerializationId::Create("min"));
        Serialization::Load(deserializer, value.max, SerializationId::Create("max"));
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::FRect>::Save(ISerializer& serializer, const SR_MATH_NS::FRect& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));
        Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));
        Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));
        Serialization::SaveCheckDefault(serializer, value.h, SerializationId::Create("h"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::FRect>::Load(IDeserializer& deserializer, SR_MATH_NS::FRect& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.x, SerializationId::Create("x"));
        Serialization::Load(deserializer, value.y, SerializationId::Create("y"));
        Serialization::Load(deserializer, value.w, SerializationId::Create("w"));
        Serialization::Load(deserializer, value.h, SerializationId::Create("h"));
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::FColor>::Save(ISerializer& serializer, const SR_MATH_NS::FColor& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.r, SerializationId::Create("r"));
        Serialization::SaveCheckDefault(serializer, value.g, SerializationId::Create("g"));
        Serialization::SaveCheckDefault(serializer, value.b, SerializationId::Create("b"));
        Serialization::SaveCheckDefault(serializer, value.a, SerializationId::Create("a"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::FColor>::Load(IDeserializer& deserializer, SR_MATH_NS::FColor& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.r, SerializationId::Create("r"));
        Serialization::Load(deserializer, value.g, SerializationId::Create("g"));
        Serialization::Load(deserializer, value.b, SerializationId::Create("b"));
        Serialization::Load(deserializer, value.a, SerializationId::Create("a"));
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::IRect>::Save(ISerializer& serializer, const SR_MATH_NS::IRect& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));
        Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));
        Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));
        Serialization::SaveCheckDefault(serializer, value.h, SerializationId::Create("h"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::IRect>::Load(IDeserializer& deserializer, SR_MATH_NS::IRect& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.x, SerializationId::Create("x"));
        Serialization::Load(deserializer, value.y, SerializationId::Create("y"));
        Serialization::Load(deserializer, value.w, SerializationId::Create("w"));
        Serialization::Load(deserializer, value.h, SerializationId::Create("h"));
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::URect>::Save(ISerializer& serializer, const SR_MATH_NS::URect& value, const SerializationId& id) {
        serializer.BeginObject(id);
        Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));
        Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));
        Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));
        Serialization::SaveCheckDefault(serializer, value.h, SerializationId::Create("h"));
        serializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::URect>::Load(IDeserializer& deserializer, SR_MATH_NS::URect& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        Serialization::Load(deserializer, value.x, SerializationId::Create("x"));
        Serialization::Load(deserializer, value.y, SerializationId::Create("y"));
        Serialization::Load(deserializer, value.w, SerializationId::Create("w"));
        Serialization::Load(deserializer, value.h, SerializationId::Create("h"));
        deserializer.EndObject();
    }

    template<uint64_t dim, typename Type> void SaveVec(ISerializer& serializer, const Type& value, const SerializationId& id) {
        serializer.BeginObject(id);
        if constexpr (dim >= 1) { Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x")); }
        if constexpr (dim >= 2) { Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y")); }
        if constexpr (dim >= 3) { Serialization::SaveCheckDefault(serializer, value.z, SerializationId::Create("z")); }
        if constexpr (dim >= 4) { Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w")); }
        if constexpr (dim >= 5) { Serialization::SaveCheckDefault(serializer, value.v, SerializationId::Create("v")); }
        if constexpr (dim >= 6) { Serialization::SaveCheckDefault(serializer, value.u, SerializationId::Create("u")); }
        serializer.EndObject();
    }

    template<uint64_t dim, typename Type> void LoadVec(IDeserializer& deserializer, Type& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }
        if constexpr (dim >= 1) { Serialization::Load(deserializer, value.x, SerializationId::Create("x")); }
        if constexpr (dim >= 2) { Serialization::Load(deserializer, value.y, SerializationId::Create("y")); }
        if constexpr (dim >= 3) { Serialization::Load(deserializer, value.z, SerializationId::Create("z")); }
        if constexpr (dim >= 4) { Serialization::Load(deserializer, value.w, SerializationId::Create("w")); }
        if constexpr (dim >= 5) { Serialization::Load(deserializer, value.v, SerializationId::Create("v")); }
        if constexpr (dim >= 6) { Serialization::Load(deserializer, value.u, SerializationId::Create("u")); }
        deserializer.EndObject();
    }

    void ObjectDataAccessor<SR_MATH_NS::FVector2>::Save(ISerializer& serializer, const SR_MATH_NS::FVector2& value, const SerializationId& id) { SaveVec<2>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::FVector2>::Load(IDeserializer& deserializer, SR_MATH_NS::FVector2& value, const SerializationId& id) { LoadVec<2>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector2>::Save(ISerializer& serializer, const SR_MATH_NS::IVector2& value, const SerializationId& id) { SaveVec<2>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector2>::Load(IDeserializer& deserializer, SR_MATH_NS::IVector2& value, const SerializationId& id) { LoadVec<2>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector2>::Save(ISerializer& serializer, const SR_MATH_NS::UVector2& value, const SerializationId& id) { SaveVec<2>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector2>::Load(IDeserializer& deserializer, SR_MATH_NS::UVector2& value, const SerializationId& id) { LoadVec<2>(deserializer, value, id); }

    void ObjectDataAccessor<SR_MATH_NS::FVector3>::Save(ISerializer& serializer, const SR_MATH_NS::FVector3& value, const SerializationId& id) { SaveVec<3>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::FVector3>::Load(IDeserializer& deserializer, SR_MATH_NS::FVector3& value, const SerializationId& id) { LoadVec<3>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::BVector3>::Save(ISerializer& serializer, const SR_MATH_NS::BVector3& value, const SerializationId& id) { SaveVec<3>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::BVector3>::Load(IDeserializer& deserializer, SR_MATH_NS::BVector3& value, const SerializationId& id) { LoadVec<3>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector3>::Save(ISerializer& serializer, const SR_MATH_NS::IVector3& value, const SerializationId& id) { SaveVec<3>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector3>::Load(IDeserializer& deserializer, SR_MATH_NS::IVector3& value, const SerializationId& id) { LoadVec<3>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector3>::Save(ISerializer& serializer, const SR_MATH_NS::UVector3& value, const SerializationId& id) { SaveVec<3>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector3>::Load(IDeserializer& deserializer, SR_MATH_NS::UVector3& value, const SerializationId& id) { LoadVec<3>(deserializer, value, id); }

    void ObjectDataAccessor<SR_MATH_NS::FVector4>::Save(ISerializer& serializer, const SR_MATH_NS::FVector4& value, const SerializationId& id) { SaveVec<4>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::FVector4>::Load(IDeserializer& deserializer, SR_MATH_NS::FVector4& value, const SerializationId& id) { LoadVec<4>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::BVector4>::Save(ISerializer& serializer, const SR_MATH_NS::BVector4& value, const SerializationId& id) { SaveVec<4>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::BVector4>::Load(IDeserializer& deserializer, SR_MATH_NS::BVector4& value, const SerializationId& id) { LoadVec<4>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector4>::Save(ISerializer& serializer, const SR_MATH_NS::IVector4& value, const SerializationId& id) { SaveVec<4>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector4>::Load(IDeserializer& deserializer, SR_MATH_NS::IVector4& value, const SerializationId& id) { LoadVec<4>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector4>::Save(ISerializer& serializer, const SR_MATH_NS::UVector4& value, const SerializationId& id) { SaveVec<4>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector4>::Load(IDeserializer& deserializer, SR_MATH_NS::UVector4& value, const SerializationId& id) { LoadVec<4>(deserializer, value, id); }

    void ObjectDataAccessor<SR_MATH_NS::FVector6>::Save(ISerializer& serializer, const SR_MATH_NS::FVector6& value, const SerializationId& id) { SaveVec<6>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::FVector6>::Load(IDeserializer& deserializer, SR_MATH_NS::FVector6& value, const SerializationId& id) { LoadVec<6>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector6>::Save(ISerializer& serializer, const SR_MATH_NS::IVector6& value, const SerializationId& id) { SaveVec<6>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::IVector6>::Load(IDeserializer& deserializer, SR_MATH_NS::IVector6& value, const SerializationId& id) { LoadVec<6>(deserializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector6>::Save(ISerializer& serializer, const SR_MATH_NS::UVector6& value, const SerializationId& id) { SaveVec<6>(serializer, value, id); }
    void ObjectDataAccessor<SR_MATH_NS::UVector6>::Load(IDeserializer& deserializer, SR_MATH_NS::UVector6& value, const SerializationId& id) { LoadVec<6>(deserializer, value, id); }
}