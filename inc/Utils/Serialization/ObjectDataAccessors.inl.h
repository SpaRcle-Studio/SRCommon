//
// Created by Monika on 17.10.2024.
//

#ifndef SR_ENGINE_OBJECT_DATA_ACCESSORS_INL_H
#define SR_ENGINE_OBJECT_DATA_ACCESSORS_INL_H

template<> struct ObjectDataAccessor<std::string> {
	static void Save(ISerializer& serializer, const std::string& value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::string& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_UTILS_NS::StringAtom> {
	static void Save(ISerializer& serializer, const SR_UTILS_NS::StringAtom& value, const SerializationId& id) ;
	static void Load(IDeserializer& deserializer, SR_UTILS_NS::StringAtom& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_HTYPES_NS::UnicodeString> {
	static void Save(ISerializer& serializer, const SR_HTYPES_NS::UnicodeString& value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, SR_HTYPES_NS::UnicodeString& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<bool> {
	static void Save(ISerializer& serializer, const bool value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, bool& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<float_t> {
	static void Save(ISerializer& serializer, const float_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, float_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<double_t> {
	static void Save(ISerializer& serializer, const double_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, double& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::int8_t> {
	static void Save(ISerializer& serializer, const std::int8_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::int8_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::int16_t> {
	static void Save(ISerializer& serializer, const std::int16_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::int16_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::int32_t> {
	static void Save(ISerializer& serializer, const std::int32_t value, const SerializationId& id) ;
	static void Load(IDeserializer& deserializer, std::int32_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::int64_t> {
	static void Save(ISerializer& serializer, const std::int64_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::int64_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::uint8_t> {
	static void Save(ISerializer& serializer, std::uint8_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::uint8_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::uint16_t> {
	static void Save(ISerializer& serializer, std::uint16_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::uint16_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::uint32_t> {
	static void Save(ISerializer& serializer, std::uint32_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::uint32_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<std::uint64_t> {
	static void Save(ISerializer& serializer, std::uint64_t value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, std::uint64_t& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::Quaternion> {
	static void Save(ISerializer& serializer, const SR_MATH_NS::Quaternion& value, const SerializationId& id);
	static void Load(IDeserializer& deserializer, SR_MATH_NS::Quaternion& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FRect> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FRect& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FRect& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FColor> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FColor& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FColor& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::IRect> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::IRect& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::IRect& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::URect> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::URect& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::URect& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FVector2> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FVector2& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FVector2& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::IVector2> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::IVector2& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::IVector2& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::UVector2> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::UVector2& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::UVector2& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FVector3> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FVector3& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FVector3& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::BVector3> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::BVector3& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::BVector3& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::IVector3> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::IVector3& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::IVector3& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::UVector3> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::UVector3& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::UVector3& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FVector4> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FVector4& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FVector4& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::BVector4> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::BVector4& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::BVector4& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::IVector4> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::IVector4& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::IVector4& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::UVector4> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::UVector4& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::UVector4& value, const SerializationId& id);
};

template<> struct ObjectDataAccessor<SR_MATH_NS::FVector6> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::FVector6& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::FVector6& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::IVector6> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::IVector6& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::IVector6& value, const SerializationId& id);
};
template<> struct ObjectDataAccessor<SR_MATH_NS::UVector6> {
    static void Save(ISerializer& serializer, const SR_MATH_NS::UVector6& value, const SerializationId& id);
    static void Load(IDeserializer& deserializer, SR_MATH_NS::UVector6& value, const SerializationId& id);
};

#endif /// SR_ENGINE_OBJECT_DATA_ACCESSORS_INL_H