//
// Created by Monika on 11.12.2025.
//

template<class T> struct ObjectDataAccessorVector {
private:
	SR_CONSTEXPR static SerializationId itemId = SerializationId::Create("i");
	SR_CONSTEXPR static SerializationId dataId = SerializationId::Create("d");

public:
	static void Save(ISerializer& serializer, const T& value, const SerializationId& id) {
		uint64_t count = 0;

		for (auto&& item : value) {
			if (SR_UTILS_NS::Serialization::CanBeSaved(item)) {
				++count;
			}
		}

		serializer.BeginArray(count, id);

        if constexpr (std::is_same_v<T, std::vector<bool>>) {
            for (uint32_t i = 0; i < value.size(); ++i) {
                serializer.BeginItem(itemId);
                const bool b = value[i];
                Serialization::Save(serializer, b, dataId);
                serializer.EndItem();
            }
        }
        else {
            for (auto &&item: value) {
                if (!SR_UTILS_NS::Serialization::CanBeSaved(item)) {
                    continue;
                }
                serializer.BeginItem(itemId);
                Serialization::Save(serializer, item, dataId);
                serializer.EndItem();
            }
        }

		serializer.EndArray();
	}

	static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
        if (!deserializer.IsPreserveMode()) {
            value.clear();
        }

		const uint64_t size = deserializer.BeginArray(id);
		if (size == 0) {
			return;
		}

		if (!deserializer.IsPreserveMode()) {
			/// already cleared
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

		while (deserializer.BeginItem(itemId, index)) {
			if (deserializer.IsPreserveMode() && index < value.size()) {
				if constexpr (std::is_same_v<T, std::vector<bool>>) {
					bool item = false;
					Serialization::Load(deserializer, item, dataId);
					value[index] = item;
				}
				else {
					Serialization::Load(deserializer, value[index], dataId);
				}
			}
			else {
				if constexpr (std::is_same_v<T, std::vector<bool>>) {
					value.emplace_back();
					bool item = false;
					Serialization::Load(deserializer, item, dataId);
					value[index] = item;
				}
				else {
					auto&& item = value.emplace_back();
					Serialization::Load(deserializer, item, dataId);
					if (!SR_UTILS_NS::Serialization::IsValidValue(item)) {
						value.pop_back();
					}
				}
			}

			deserializer.EndItem();
			index++;
		}

		deserializer.EndArray();
	}
};

template<typename T> struct ObjectDataAccessor<SR_UTILS_NS::Vector<T>> : ObjectDataAccessorVector<SR_UTILS_NS::Vector<T>> { };

template<typename T, size_t N> struct ObjectDataAccessor<std::array<T, N>> {
	static void Save(ISerializer& serializer, const std::array<T, N>& value, const SerializationId& id) {
		uint64_t count = 0;

		for (auto&& item : value) {
			if (SR_UTILS_NS::Serialization::CanBeSaved(item)) {
				++count;
			}
		}

		serializer.BeginArray(count, id);

		for (uint64_t i = 0; i < value.size(); ++i) {
			const auto& item = value[i];

			if (!SR_UTILS_NS::Serialization::CanBeSaved(item)) {
				continue;
			}

			if (IsValidValue(item) && (serializer.IsWriteDefaults() || !IsDefault(item))) {
				constexpr auto itemId = SerializationId::Create("item");
				constexpr auto dataId = SerializationId::Create("data");

				serializer.BeginItem(itemId);

				Serialization::Save(serializer, item, dataId);

				serializer.EndItem();
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

		/*while (deserializer.NextItem(id)) {
			deserializer.BeginObject(id);

			if (index < N) {
				Serialization::Load(deserializer, value[index], SerializationId::Create("item"));
				++index;
			}
			else {
				SRHalt("Array index out of range!");
			}
			deserializer.EndObject();
		}*/

		deserializer.EndArray();
	}
};

template<typename MapType, typename T, typename U> struct ObjectDataAccessorMap {
	static_assert(std::is_arithmetic_v<T> || IsSREnumV<T> || std::is_same_v<T, SR_UTILS_NS::String> || std::is_same_v<T, SR_UTILS_NS::StringAtom>, "Custom types and structs are not supported as id of map");

    static constexpr SerializationId itemId = SerializationId::Create("item");
    static constexpr SerializationId firstId = SerializationId::Create("first");
    static constexpr SerializationId secondId = SerializationId::Create("second");

	static void Save(ISerializer& serializer, const MapType& value, const SerializationId& id) {
        uint64_t count = 0;

        SR_THREAD_LOCAL static SR_UTILS_NS::Vector<SR_UTILS_NS::Pair<const T*, const U*>> tempVector;
        tempVector.clear();
        for (auto&& item : value) {
            if (SR_UTILS_NS::Serialization::CanBeSaved(item)) {
                ++count;
                tempVector.emplace_back(&item.first, &item.second);
            }
        }

        serializer.BeginArray(count, id);

        std::stable_sort(tempVector.begin(), tempVector.end(), [](const SR_UTILS_NS::Pair<const T*, const U*>& a, const SR_UTILS_NS::Pair<const T*, const U*>& b) {
            return *a.first < *b.first;
        });

        for (auto&& item : tempVector) {
            serializer.BeginItem(itemId);

            Serialization::Save(serializer, *item.first, firstId);
            Serialization::Save(serializer, *item.second, secondId);

            serializer.EndItem();
        }

		serializer.EndArray();
	}

	template<typename MapT>
	static void Load(IDeserializer& deserializer, MapT& value, const SerializationId& id) {
        if (!deserializer.IsPreserveMode()) {
            value.clear();
        }

		const uint64_t size = deserializer.BeginArray(id);
        if (size == 0) {
            return;
        }

        if constexpr (SR_UTILS_NS::IsDetectedV<Details::ReserveMethodT, MapT>) {
            value.reserve(size);
        }

        uint64_t index = 0;

        while (deserializer.BeginItem(itemId, index)) {
            if (deserializer.IsPreserveMode() && index < value.size()) {
                T key = {};
                Serialization::Load(deserializer, key, firstId);
                auto it = value.find(key);
                if (it != value.end()) {
                    Serialization::Load(deserializer, it->second, secondId);
                }
                else if (deserializer.AllowNewMapKeys()) {
                    U itemValue = {};
                    Serialization::Load(deserializer, itemValue, secondId);
                    if (SR_UTILS_NS::Serialization::IsValidValue(key) && SR_UTILS_NS::Serialization::IsValidValue(itemValue)) {
                        value.emplace(std::move(key), std::move(itemValue));
                    }
                }
            }
            else {
                Pair<T, U> pair;

                Serialization::Load(deserializer, pair.first, firstId);
                Serialization::Load(deserializer, pair.second, secondId);

                if (SR_UTILS_NS::Serialization::IsValidValue(pair.first) && SR_UTILS_NS::Serialization::IsValidValue(pair.second)) {
                    value.insert(std::make_pair(std::move(pair.first), std::move(pair.second)));
                }
            }

            deserializer.EndItem();
            index++;
        }

		deserializer.EndArray();
	}
};

template<typename T, typename U, typename Compare>
struct ObjectDataAccessor<Map<T, U, Compare>> : public ObjectDataAccessorMap<Map<T, U, Compare>, T, U> { };

template<typename T, typename U>
struct ObjectDataAccessor<SR_HTYPES_NS::FlatHashMap<T, U>> : public ObjectDataAccessorMap<SR_HTYPES_NS::FlatHashMap<T, U>, T, U> { };

template<typename T, typename Less>
struct ObjectDataAccessor<Set<T, Less>> {
private:
    SR_CONSTEXPR static SerializationId itemId = SerializationId::Create("i");
    SR_CONSTEXPR static SerializationId dataId = SerializationId::Create("d");

public:
	using SetType = Set<T, Less>;

    static void Save(ISerializer& serializer, const SetType& value, const SerializationId& id) {
        uint64_t count = 0;

        for (auto&& item : value) {
            if (SR_UTILS_NS::Serialization::CanBeSaved(item)) {
                ++count;
            }
        }

        serializer.BeginArray(count, id);

        for (auto&& item : value) {
            if (!SR_UTILS_NS::Serialization::CanBeSaved(item)) {
                continue;
            }
            serializer.BeginItem(itemId);
            Serialization::Save(serializer, item, dataId);
            serializer.EndItem();
        }

        serializer.EndArray();
    }

    static void Load(IDeserializer& deserializer, SetType& value, const SerializationId& id) {
        if (!deserializer.IsPreserveMode()) {
            value.clear();
        }

        const uint64_t size = deserializer.BeginArray(id);
        if (size == 0) {
            return;
        }

        uint64_t index = 0;

        while (deserializer.BeginItem(itemId, index)) {
            auto&& item = T();
            Serialization::Load(deserializer, item, dataId);
            if (SR_UTILS_NS::Serialization::IsValidValue(item)) {
                value.insert(std::move(item));
            }

            deserializer.EndItem();
            index++;
        }

        deserializer.EndArray();
    }
};

template<typename T, typename U> struct ObjectDataAccessor<SR_UTILS_NS::Pair<T, U>> {
    static constexpr SerializationId firstId = SerializationId::Create("first");
    static constexpr SerializationId secondId = SerializationId::Create("second");

	static void Save(ISerializer& serializer, const SR_UTILS_NS::Pair<T, U>& value, const SerializationId& id) {
		serializer.BeginObject(id);
		Serialization::Save(serializer, value.first, firstId);
		Serialization::Save(serializer, value.second, secondId);
		serializer.EndObject();
	}

	static void Load(IDeserializer& deserializer, SR_UTILS_NS::Pair<T, U>& value, const SerializationId& id) {
		if (!deserializer.BeginObject(id)) {
			return;
		}

		Serialization::Load(deserializer, value.first, firstId);
		Serialization::Load(deserializer, value.second, secondId);

		deserializer.EndObject();
	}
};

template<typename T> struct ObjectDataAccessor<SR_UTILS_NS::Optional<T>> {
    static constexpr SerializationId hasValueId = SerializationId::Create("has");

    static void Save(ISerializer& serializer, const SR_UTILS_NS::Optional<T>& value, const SerializationId& id) {
        serializer.BeginObject(id);
        if (value.has_value()) {
            serializer.WriteBool(true, hasValueId);
            Serialization::Save(serializer, value.value(), SerializationId::Create("value"));
        }
        else {
            serializer.WriteBool(false, hasValueId);
        }
        serializer.EndObject();
    }

    static void Load(IDeserializer& deserializer, SR_UTILS_NS::Optional<T>& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }

        bool hasValue = false;
        deserializer.ReadBool(hasValue, hasValueId);

        if (hasValue) {
            T item = T();
            Serialization::Load(deserializer, item, SerializationId::Create("value"));
            value = std::move(item);
        }
        else {
            value.reset();
        }

        deserializer.EndObject();
    }
};

template<typename T> struct ObjectDataAccessor<T, typename std::enable_if<IsSREnumV<T>>::type> {
    static void Save(ISerializer& serializer, T value, const SerializationId& id) {
        serializer.WriteString(SR_UTILS_NS::EnumReflector::ToStringAtom<T>(value).ToStringView(), id);
    }

    static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
        SR_UTILS_NS::String enumName;
        deserializer.ReadString(enumName, id);
        if (enumName.empty()) {
            return; // Default value
        }
        if (!SR_UTILS_NS::EnumReflector::FromString<T>(enumName.c_str(), value)) {
            deserializer.ReportError("Invalid enum value \"" + enumName + " for type: {}, id: {}"_format(typeid(T).name(), id.GetName()));
        }
    }
};

template<typename T>
struct ObjectDataAccessor<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<SerializationTraits<T>::IsSerializable>> {
    static constexpr SerializationId TYPE_ID = SerializationId::Create("type");
    static constexpr SerializationId PTR_ID = SerializationId::Create("ptr");

    static void Save(ISerializer& serializer, const SR_HTYPES_NS::SharedPtr<T>& value, const SerializationId& id) {
        if (!value) {
            return;
        }

        if (value->GetMeta()->IsEditorOnly()) {
            if (!serializer.IsEditorAllowed()) {
                return;
            }
        }

        const SRClassMeta* pMeta = value->GetMeta();
        auto&& typeName = pMeta->GetFactoryName();

        if (pMeta->IsAbstract()) {
            SR_ERROR("ObjectDataAccessor::Save() : abstract class can't be saved! Factory name: {}", typeName);
            return;
        }

        serializer.BeginObject(id);

        serializer.WriteString(typeName, TYPE_ID);

        Serialization::Save(serializer, *value, PTR_ID);

        serializer.EndObject();
    }

    static void Load(IDeserializer& deserializer, SR_HTYPES_NS::SharedPtr<T>& value, const SerializationId& id) {
        if (!deserializer.BeginObject(id)) {
            return;
        }

        SR_UTILS_NS::String type;
        deserializer.ReadString(type, TYPE_ID);

        if (deserializer.IsDefault(PTR_ID)) {
            deserializer.EndObject();
            return;
        }

        if (!type.empty()) {
            if (!deserializer.IsPreserveMode()) {
                value = SR_UTILS_NS::Factory::Instance().Create<T>(type);
                if (!value) {
                    SR_ERROR("ObjectDataAccessor::Load() : unknown object's type: {}", type);
                }
            }
            else {
                const bool isNeedReAlloc = !value || type != SR_UTILS_NS::Factory::Instance().GetName(value.Get());
                if (isNeedReAlloc && deserializer.AllowReAllocPointer(value ? IDeserializer::ReAllocPointerReason::HasDifferentType : IDeserializer::ReAllocPointerReason::IsNull)) {
                    value = SR_UTILS_NS::Factory::Instance().Create<T>(type);
                    if (!value) {
                        SR_ERROR("ObjectDataAccessor::Load() : unknown object's type: {}", type);
                    }
                }
            }
        }

        if (value) {
            Serialization::Load(deserializer, *value, PTR_ID);

            SR_UTILS_NS::SerializableVerifyContext context;
            value->VerifyAfterLoad(context);

            for (auto&& warning : context.GetWarnings()) {
                SR_WARN("ObjectDataAccessor::Load() : warning: {}", warning);
            }
            for (auto&& error : context.GetErrors()) {
                SR_ERROR("ObjectDataAccessor::Load() : error: {}", error);
            }
        }

        deserializer.EndObject();
    }
};

template<typename T> struct ObjectDataAccessor<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<!SerializationTraits<T>::HasOriginType>> {
    static void Load(ISerializer&, SR_HTYPES_NS::SharedPtr<T>&, const SerializationId&) {
        static_assert(std::is_same_v<void, T>, "Serialization of pointers without T::OriginType error prone and forbidden!");
    }

    static void Save(IDeserializer&, const SR_HTYPES_NS::SharedPtr<T>&, const SerializationId&) {
        static_assert(std::is_same_v<void, T>, "Serialization of pointers without T::OriginType error prone and forbidden!");
    }
};
