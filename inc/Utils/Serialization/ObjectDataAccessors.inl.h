//
// Created by Monika on 17.10.2024.
//

template<> struct ObjectDataAccessor<std::string> {
	static void Save(ISerializer& serializer, const std::string& value, const SerializationId& id) {
		serializer.WriteString(value, id);
	}

	static void Load(IDeserializer& deserializer, std::string& value, const SerializationId& id) {
		deserializer.ReadString(value, id);
	}
};

template<> struct ObjectDataAccessor<SR_UTILS_NS::StringAtom> {
	static void Save(ISerializer& serializer, const SR_UTILS_NS::StringAtom& value, const SerializationId& id) {
		serializer.WriteString(value.ToStringView(), id);
	}

	static void Load(IDeserializer& deserializer, SR_UTILS_NS::StringAtom& value, const SerializationId& id) {
		deserializer.ReadString(value, id);
	}
};

template<> struct ObjectDataAccessor<bool> {
	static void Save(ISerializer& serializer, const bool value, const SerializationId& id) {
		serializer.WriteBool(value, id);
	}

	static void Load(IDeserializer& deserializer, bool& value, const SerializationId& id) {
		deserializer.ReadBool(value, id);
	}
};

template<> struct ObjectDataAccessor<float_t> {
	static void Save(ISerializer& serializer, const float_t value, const SerializationId& id) {
		serializer.WriteFloat(value, id);
	}

	static void Load(IDeserializer& deserializer, float_t& value, const SerializationId& id) {
		deserializer.ReadFloat(value, id);
	}
};

template<> struct ObjectDataAccessor<double_t> {
	static void Save(ISerializer& serializer, const double_t value, const SerializationId& id) {
		serializer.WriteDouble(value, id);
	}

	static void Load(IDeserializer& deserializer, double& value, const SerializationId& id) {
		deserializer.ReadDouble(value, id);
	}
};

template<> struct ObjectDataAccessor<std::int8_t> {
	static void Save(ISerializer& serializer, const std::int8_t value, const SerializationId& id) {
		serializer.WriteInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::int8_t& value, const SerializationId& id) {
		deserializer.ReadInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::int16_t> {
	static void Save(ISerializer& serializer, const std::int16_t value, const SerializationId& id) {
		serializer.WriteInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::int16_t& value, const SerializationId& id) {
		deserializer.ReadInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::int32_t> {
	static void Save(ISerializer& serializer, const std::int32_t value, const SerializationId& id) {
		serializer.WriteInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::int32_t& value, const SerializationId& id) {
		deserializer.ReadInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::int64_t> {
	static void Save(ISerializer& serializer, const std::int64_t value, const SerializationId& id) {
		serializer.WriteInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::int64_t& value, const SerializationId& id) {
		deserializer.ReadInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::uint8_t> {
	static void Save(ISerializer& serializer, std::uint8_t value, const SerializationId& id) {
		serializer.WriteUInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::uint8_t& value, const SerializationId& id) {
		deserializer.ReadUInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::uint16_t> {
	static void Save(ISerializer& serializer, std::uint16_t value, const SerializationId& id) {
		serializer.WriteUInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::uint16_t& value, const SerializationId& id) {
		deserializer.ReadUInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::uint32_t> {
	static void Save(ISerializer& serializer, std::uint32_t value, const SerializationId& id) {
		serializer.WriteUInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::uint32_t& value, const SerializationId& id) {
		deserializer.ReadUInt(value, id);
	}
};

template<> struct ObjectDataAccessor<std::uint64_t> {
	static void Save(ISerializer& serializer, std::uint64_t value, const SerializationId& id) {
		serializer.WriteUInt(value, id);
	}

	static void Load(IDeserializer& deserializer, std::uint64_t& value, const SerializationId& id) {
		deserializer.ReadUInt(value, id);
	}
};

template<class T> struct ObjectDataAccessorVector {
private:
	using ConstRef = std::conditional_t<std::is_same_v<typename T::value_type, bool>, bool, typename T::const_reference>;

public:
	static void Save(ISerializer& serializer, const T& value, const SerializationId& id) {
		serializer.BeginArray(value.size(), id);

		SR_CONSTEXPR SerializationId itemId = SerializationId::Create("item");
		for (ConstRef item : value) {
			Serialization::Save(serializer, item, itemId);
		}

		serializer.EndArray();
	}

	static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
		const uint64_t size = deserializer.BeginArray(id);

		if (!deserializer.IsPreserveMode()) {
			value.clear();
		}
		else if (deserializer.ShouldSetDefaults()) {
			auto it = value.end();
			uint32_t i = static_cast<uint32_t>(value.size());
			while (it != value.begin()) {
				--it;
				--i;

				const fmt::format_int index(i);
				if (deserializer.ShouldSetDefaults(SerializationId::CreateFromCStr(index.c_str()))) {
					it = value.erase(it);
				}
			}
		}

		if (SRVerify2(size != static_cast<size_t>(-1), "IDeserializer mustn't ever return -1 as number of elements!")) {
			value.reserve(size);
		}
		uint64_t index = 0;

		while (deserializer.NextItem(id)) {
			/// Try to load exists item in PreserveMode
			if (deserializer.IsPreserveMode() && index < value.size()) {
				LoadItem(deserializer, value[index], id);
			}
			else {
				auto&& item = value.emplace_back();
				LoadItem(deserializer, item, id);
				if (!SR_UTILS_NS::Serialization::IsValidValue(item)) {
					value.pop_back();
				}
			}

			index++;
		}

		deserializer.EndArray();
	}

private:
	static void LoadItem(IDeserializer& deserializer, typename T::reference item, const SerializationId& id) {
		if constexpr (!std::is_same_v<typename T::value_type, bool>) {
			Serialization::Load(deserializer, item, id);
		}
		else {
			bool itemToLoad = item;
			Serialization::Load(deserializer, itemToLoad, id);
			item = itemToLoad;
		}
	}
};

template<typename T, class... TOther>
struct ObjectDataAccessor<std::vector<T, TOther...>> : ObjectDataAccessorVector<std::vector<T, TOther...>>
{
};

template<typename T, size_t N> struct ObjectDataAccessor<std::array<T, N>> {
	static void Save(ISerializer& serializer, const std::array<T, N>& value, const SerializationId& id) {
		serializer.BeginArray(value.size(), id);

		for (uint64_t i = 0; i < value.size(); ++i) {
			const auto& item = value[i];
			if (IsValidValue(item) && (serializer.IsWriteDefaults() || !IsDefault(item))) {
				serializer.BeginObject(id);

				Serialization::Save(serializer, item, SerializationId::Create("item"));

				serializer.EndObject();
			}
		}

		serializer.EndArray();
	}

	static void Load(IDeserializer& deserializer, std::array<T, N>& value, const SerializationId& id) {
		deserializer.BeginArray(id);

		if (deserializer.IsPreserveMode() && deserializer.ShouldSetDefaults()) {
			for (size_t i = 0; i < value.size(); ++i) {
				const fmt::format_int index(i);
				if (deserializer.ShouldSetDefaults(SerializationId::CreateFromCStr(index.c_str()))) {
					if constexpr (DefaultObjectMaker<T>::value) {
						DefaultObjectMaker<T>::MakeDefault(value[i]);
					}
					else if constexpr (std::is_default_constructible_v<T>) {
						value[i] = T();
					}
				}
			}
		}

		std::uint64_t index = 0;

		while (deserializer.NextItem(id)) {
			deserializer.BeginObject(id);

			if (index < N) {
				Serialization::Load(deserializer, value[index], SerializationId::Create("item"));
				++index;
			}
			else {
				SRHalt("Array index out of range!");
			}
			deserializer.EndObject();
		}

		deserializer.EndArray();
	}
};

template<typename T, typename U, typename Compare, typename Allocator>
struct ObjectDataAccessor<std::map<T, U, Compare, Allocator>> {
	using MapType = std::map<T, U, Compare, Allocator>;
	using ValueType = typename MapType::value_type;

	static_assert(std::is_arithmetic_v<T> || IsSREnumV<T> || std::is_same_v<T, std::string> || std::is_same_v<T, SR_UTILS_NS::StringAtom>, "Custom types and structs are not supported as id of map");

	static void Save(ISerializer& serializer, const MapType& value, const SerializationId& id) {
		serializer.BeginArray(value.size(), id);

		for (auto&& item : value) {
			SR_CONSTEXPR SerializationId itemId = SerializationId::Create("item");
			serializer.BeginObject(itemId);

			Serialization::Save(serializer, item.first, SerializationId::Create("first"));
			Serialization::Save(serializer, item.second, SerializationId::Create("second"));

			serializer.EndObject();
		}

		serializer.EndArray();
	}

	template<typename MapT>
	static void Load(IDeserializer& deserializer, MapT& value, const SerializationId& id) {
		const uint64_t size = deserializer.BeginArray(id);

		if (!deserializer.IsPreserveMode()) {
			value.clear();
		}
		else if (deserializer.ShouldSetDefaults()) {
			for (auto it = value.begin(); it != value.end();) {
				if (deserializer.ShouldSetDefaults(SerializationId(it->first.c_str(), 0))) {
					it = value.erase(it);
				}
				else {
					++it;
				}
			}
		}

		if constexpr (SR_UTILS_NS::IsDetectedV<Details::ReserveMethodT, MapT>) {
			value.reserve(size + value.size());
		}

		while (deserializer.NextItem(id)) {
			if (deserializer.IsPreserveMode()) {
				deserializer.BeginObject(id);
				T element = {};
				Serialization::Load(deserializer, element, SerializationId::Create("first"));
				auto&& it = value.find(element);
				if (it != value.end()) {
					Serialization::Load(deserializer, it->second, SerializationId::Create("second"));
				}
				else if (deserializer.AllowNewMapKeys()) {
					U itemValue = {};
					Serialization::Load(deserializer, itemValue, SerializationId::Create("second"));
					if (IsValidValue(element) && IsValidValue(itemValue)) {
						value.emplace(std::move(element), std::move(itemValue));
					}
				}
				deserializer.EndObject();
			}
			else {
				std::pair<T, U> pair;

				Serialization::Load(deserializer, pair, id);

				if (IsValidValue(pair.first) && IsValidValue(pair.second)) {
					value.insert(std::move(pair));
				}
			}
		}

		deserializer.EndArray();
	}
};

template<typename T, typename Less, typename Allocator>
struct ObjectDataAccessor<std::set<T, Less, Allocator>> {
	using SetType = std::set<T, Less, Allocator>;
	using ValueType = typename SetType::value_type;

	static void Save(ISerializer& serializer, const SetType& value, const SerializationId& id) {
		serializer.BeginArray(value.size(), id);

		for (auto&& item : value) {
			SR_CONSTEXPR SerializationId itemId = SerializationId::Create("item");
			Serialization::Save(serializer, item.second, itemId);
		}

		serializer.EndArray();
	}

	template<typename SetT>
	static void Load(IDeserializer& deserializer, SetT& value, const SerializationId& id) {
		const uint64_t size = deserializer.BeginArray(id);

		if (!deserializer.IsPreserveMode()) {
			value.clear();
		}
		else if (deserializer.ShouldSetDefaults()) {
			for (auto it = value.begin(); it != value.end();) {
				if (deserializer.ShouldSetDefaults(SerializationId(it->first.c_str(), 0))) {
					it = value.erase(it);
				}
				else {
					++it;
				}
			}
		}

		if constexpr (SR_UTILS_NS::IsDetectedV<Details::ReserveMethodT, SetT>) {
			value.reserve(size + value.size());
		}

		while (deserializer.NextItem(id)) {
			if (deserializer.IsPreserveMode()) {
				deserializer.BeginObject(id);
				T item = {};
				Serialization::Load(deserializer, item, SerializationId::Create("item"));
				if (IsValidValue(item)) {
					value.insert(std::move(item));
				}
				deserializer.EndObject();
			}
			else {
				T item;

				Serialization::Load(deserializer, item, id);

				if (IsValidValue(item)) {
					value.insert(std::move(item));
				}
			}
		}

		deserializer.EndArray();
	}
};

template<typename T, typename U> struct ObjectDataAccessor<std::pair<T, U>> {
	static void Save(ISerializer& serializer, const std::pair<T, U>& value, const SerializationId& id) {
		serializer.BeginObject(id);
		Serialization::Save(serializer, value.first, SerializationId::Create("first"));
		Serialization::Save(serializer, value.second, SerializationId::Create("second"));
		serializer.EndObject();
	}

	static void Load(IDeserializer& deserializer, std::pair<T, U>& value, const SerializationId& id) {
		deserializer.BeginObject(id);

		Serialization::Load(deserializer, value.first, SerializationId::Create("first"));
		Serialization::Load(deserializer, value.second, SerializationId::Create("second"));

		deserializer.EndObject();
	}
};

#define SR_DATA_ACCESSOR_VECTOR2(Type)																					\
template<> struct ObjectDataAccessor<Type> {																			\
	static void Save(ISerializer& serializer, const Type& value, const SerializationId& id) {							\
		serializer.BeginObject(id);																						\
		Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));								\
		Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));								\
		serializer.EndObject();																							\
	}																													\
																														\
	static void Load(IDeserializer& deserializer, Type& value, const SerializationId& id) {								\
		deserializer.BeginObject(id);																					\
		Serialization::Load(deserializer, value.x, SerializationId::Create("x"));										\
		Serialization::Load(deserializer, value.y, SerializationId::Create("y"));										\
		deserializer.EndObject();																						\
	}							 																						\
};								 																						\


#define SR_DATA_ACCESSOR_VECTOR3(Type)																					\
template<> struct ObjectDataAccessor<Type> {																			\
	static void Save(ISerializer& serializer, const Type& value, const SerializationId& id) {							\
		serializer.BeginObject(id);																						\
		Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));								\
		Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));								\
		Serialization::SaveCheckDefault(serializer, value.z, SerializationId::Create("z"));								\
		serializer.EndObject();																							\
	}																													\
																														\
	static void Load(IDeserializer& deserializer, Type& value, const SerializationId& id) {								\
		deserializer.BeginObject(id);																					\
		Serialization::Load(deserializer, value.x, SerializationId::Create("x"));										\
		Serialization::Load(deserializer, value.y, SerializationId::Create("y"));										\
		Serialization::Load(deserializer, value.z, SerializationId::Create("z"));										\
		deserializer.EndObject();																						\
	}							 																						\
};								 																						\


#define SR_DATA_ACCESSOR_VECTOR4(Type)																					\
template<> struct ObjectDataAccessor<Type> {																			\
	static void Save(ISerializer& serializer, const Type& value, const SerializationId& id) {							\
		serializer.BeginObject(id);																						\
		Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));								\
		Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));								\
		Serialization::SaveCheckDefault(serializer, value.z, SerializationId::Create("z"));								\
		Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));								\
		serializer.EndObject();																							\
	}																													\
																														\
	static void Load(IDeserializer& deserializer, Type& value, const SerializationId& id) {								\
		deserializer.BeginObject(id);																					\
		Serialization::Load(deserializer, value.x, SerializationId::Create("x"));										\
		Serialization::Load(deserializer, value.y, SerializationId::Create("y"));										\
		Serialization::Load(deserializer, value.z, SerializationId::Create("z"));										\
		Serialization::Load(deserializer, value.w, SerializationId::Create("w"));										\
		deserializer.EndObject();																						\
	}							 																						\
};								 																						\

SR_DATA_ACCESSOR_VECTOR2(SR_MATH_NS::FVector2)
SR_DATA_ACCESSOR_VECTOR2(SR_MATH_NS::BVector2)
SR_DATA_ACCESSOR_VECTOR2(SR_MATH_NS::IVector2)
SR_DATA_ACCESSOR_VECTOR2(SR_MATH_NS::UVector2)

SR_DATA_ACCESSOR_VECTOR3(SR_MATH_NS::FVector3)
SR_DATA_ACCESSOR_VECTOR3(SR_MATH_NS::BVector3)
SR_DATA_ACCESSOR_VECTOR3(SR_MATH_NS::IVector3)
SR_DATA_ACCESSOR_VECTOR3(SR_MATH_NS::UVector3)

SR_DATA_ACCESSOR_VECTOR4(SR_MATH_NS::FVector4)
SR_DATA_ACCESSOR_VECTOR4(SR_MATH_NS::BVector4)
SR_DATA_ACCESSOR_VECTOR4(SR_MATH_NS::IVector4)
SR_DATA_ACCESSOR_VECTOR4(SR_MATH_NS::UVector4)

template<> struct ObjectDataAccessor<SR_MATH_NS::Quaternion> {
	static void Save(ISerializer& serializer, const SR_MATH_NS::Quaternion& value, const SerializationId& id) {
		serializer.BeginObject(id);
		Serialization::SaveCheckDefault(serializer, value.x, SerializationId::Create("x"));
		Serialization::SaveCheckDefault(serializer, value.y, SerializationId::Create("y"));
		Serialization::SaveCheckDefault(serializer, value.z, SerializationId::Create("z"));
		Serialization::SaveCheckDefault(serializer, value.w, SerializationId::Create("w"));
		serializer.EndObject();
	}

	static void Load(IDeserializer& deserializer, SR_MATH_NS::Quaternion& value, const SerializationId& id) {
		deserializer.BeginObject(id);
		Serialization::Load(deserializer, value.x, SerializationId::Create("x"));
		Serialization::Load(deserializer, value.y, SerializationId::Create("y"));
		Serialization::Load(deserializer, value.z, SerializationId::Create("z"));
		Serialization::Load(deserializer, value.w, SerializationId::Create("w"));
		deserializer.EndObject();
	}
};

template<typename T> struct ObjectDataAccessor<std::optional<T>> {
	static void Save(ISerializer& serializer, const std::optional<T>& value, const SerializationId& id) {
		if (value.is_initialized()) {
			Serialization::Save(serializer, value.get(), id);
		}
	}
	static void Load(IDeserializer& deserializer, std::optional<T>& value, const SerializationId& id) {
		if (!value.is_initialized()) {
			value = T();
		}
		Serialization::Load(deserializer, value.get(), id);
	}
};

template<typename T>
struct ObjectDataAccessor<T, typename std::enable_if<IsSREnumV<T>>::type> {
	static void Save(ISerializer& serializer, T value, const SerializationId& id) {
		serializer.WriteString(SR_UTILS_NS::EnumReflector::ToStringAtom<T>(value).ToStringView(), id);
	}

	static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
		std::string enumName;
		deserializer.ReadString(enumName, id);
		if (!SR_UTILS_NS::EnumReflector::FromString<T>(enumName.c_str(), value)) {
			deserializer.ReportError(id, "Invalid enum value \"" + enumName + " for type: " + typeid(T).name());
		}
	}
};

template<typename T>
struct ObjectDataAccessor<T, typename std::enable_if<SerializationTraits<T>::IsSerializable>::type> {
	static void Save(ISerializer& serializer, const T& value, const SerializationId& id) {
		serializer.BeginObject(id);
		static_cast<const Serializable&>(value).Save(serializer);
		serializer.EndObject();
	}

	static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
		deserializer.BeginObject(id);
		static_cast<Serializable&>(value).Load(deserializer);
		deserializer.EndObject();
	}
};

template<typename T>
struct ObjectDataAccessor<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<SerializationTraits<T>::HasOriginType>> {
	using ObjectFactoryType = SR_UTILS_NS::Factory<typename T::OriginType>;

	static void Save(ISerializer& serializer, const SR_HTYPES_NS::SharedPtr<T>& value, const SerializationId& id) {
		if (!value) {
			return;
		}

		if (value->GetMeta()->IsEditorOnly()) {
			if (!serializer.IsEditorAllowed()) {
				return;
			}
		}

		auto typeName = value->GetMeta()->GetFactoryName();
		serializer.BeginObject(id);

		serializer.WriteString(typeName, SerializationId::Create("type"));
		Serialization::Save(serializer, *value, SerializationId::Create("ptr"));

		serializer.EndObject();
	}

	static void Load(IDeserializer& deserializer, SR_HTYPES_NS::SharedPtr<T>& value, const SerializationId& id)
	{
		deserializer.BeginObject(id);
		std::string type;
		deserializer.ReadString(type, SerializationId::Create("type"));

		if (deserializer.IsDefault(SerializationId::Create("ptr"))) {
			deserializer.EndObject();
			return;
		}
		if (!type.empty()) {
			if (!deserializer.IsPreserveMode()) {
				if constexpr (std::is_same_v<T, typename T::OriginType>) {
					value = ObjectFactoryType::Instance().Create(type);
				}
				else {
					value = ObjectFactoryType::Instance().Create(type).template DynamicCast<T>();
				}
				SRAssert2(value, "Unknown object's type: " + type);
			}
			else {
				const bool isNeedReAlloc = !value || type != ObjectFactoryType::Instance().GetName(value.Get());
				if (isNeedReAlloc && deserializer.AllowReAllocPointer(value ? IDeserializer::ReAllocPointerReason::HasDifferentType : IDeserializer::ReAllocPointerReason::IsNull)) {
					if constexpr (std::is_same_v<T, typename T::OriginType>) {
						value = ObjectFactoryType::Instance().Create(type);
					}
					else {
						value = ObjectFactoryType::Instance().Create(type).template DynamicCast<T>();
					}
					SRAssert2(value, "Unknown object's type: " + type);
				}
			}
		}

		if (value) {
			Serialization::Load(deserializer, *value, SerializationId::Create("ptr"));
		}

		deserializer.EndObject();
	}
};

template<typename T>
struct ObjectDataAccessor<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<!SerializationTraits<T>::HasOriginType>> {
	static void Load(ISerializer&, SR_HTYPES_NS::SharedPtr<T>&, const SerializationId&) {
		static_assert(std::is_same_v<void, T>, "Serialization of pointers without T::OriginType error prone and forbidden!");
	}

	static void Save(IDeserializer&, const SR_HTYPES_NS::SharedPtr<T>&, const SerializationId&) {
		static_assert(std::is_same_v<void, T>, "Serialization of pointers without T::OriginType error prone and forbidden!");
	}
};
