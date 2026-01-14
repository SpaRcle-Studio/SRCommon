//
// Created by Monika on 02.02.2025.
//

#include <Utils/Common/StoreUtils.h>
#include <Utils/Common/ToString.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Resources/Xml.h>

namespace SR_UTILS_NS::StoreUtils {
    void Storage::Save() {
        SR_TRACY_ZONE;
        SR_UTILS_NS::Path path = SR_UTILS_NS::ResourceManager::Instance().GetCachePath().Concat("UserData.xml");

        if (!path.CreateIfNotExists()) {
            SRHalt("Storage::Save() : failed to create file \"{}\"!", path);
        }

        auto&& document = SR_XML_NS::Document::New();
        auto&& root = document.Root().AppendChild("UserData");
        auto&& pUserDataIt = m_storage.find(StorageType::User);
        if (pUserDataIt != m_storage.end()) {
            for (auto&& [key, value] : pUserDataIt->second) {
                auto&& node = root.AppendChild(key.ToString());
                switch (value.type) {
                case ValueType::Float:
                    node.AppendAttribute("Type", "Float");
                    node.AppendAttribute("Value", value.value.f);
                    break;
                case ValueType::Bool:
                    node.AppendAttribute("Type", "Bool");
                    node.AppendAttribute("Value", value.value.b);
                    break;
                case ValueType::Int:
                    node.AppendAttribute("Type", "Int");
                    node.AppendAttribute("Value", value.value.i);
                    break;
                case ValueType::String:
                    node.AppendAttribute("Type", "String");
                    node.AppendAttribute("Value", *value.value.s);
                    break;
                default:
                    SRHalt("Storage::Save() : unsupported value type!");
                    break;
                }
            }
        }

        if (!document.Save(path)) {
            SRHalt("Storage::Save() : failed to save YAML document to file \"{}\"!", path);
        }
        else {
            SR_INFO("Storage::Save() : user data saved to \"{}\"!", path);
        }
    }

    void Storage::Load() {
        SR_TRACY_ZONE;
        SR_UTILS_NS::Path path = SR_UTILS_NS::ResourceManager::Instance().GetCachePath().Concat("UserData.xml");

        if (!path.Exists(SR_UTILS_NS::Path::Type::File)) {
            return;
        }

        auto&& document = SR_XML_NS::Document::Load(path);
        auto&& root = document.Root().GetNode("UserData");
        if (!root) {
            return;
        }

        for (auto&& node : root.GetNodes()) {
            auto&& type = node.GetAttribute("Type").ToString();

            if (type == "Float") {
                Set(StorageType::User, ValueType::Float, node.Name(), node.GetAttribute("Value").ToFloat());
            }
            else if (type == "Bool") {
                Set(StorageType::User, ValueType::Bool, node.Name(), node.GetAttribute("Value").ToBool());
            }
            else if (type == "Int") {
                Set(StorageType::User, ValueType::Int, node.Name(), node.GetAttribute("Value").ToInt64());
            }
            else if (type == "String") {
                Set(StorageType::User, ValueType::String, node.Name(), node.GetAttribute("Value").ToString());
            }
            else {
                SRHalt("Storage::Load() : unsupported value type!");
            }
        }
    }

    bool Storage::Has(const StorageType storageTepe, ValueType valueType, const StringAtom key)
    {
        SR_TRACY_ZONE;
        if (auto&& pStorageIt = m_storage.find(storageTepe); pStorageIt != m_storage.end()) {
            if (auto&& pValueIt = pStorageIt->second.find(key); pValueIt != pStorageIt->second.end()) {
                return true;
            }
        }
        return false;
    }

    bool Storage::Drop(const StorageType storageTepe, const StringAtom key)
    {
        SR_TRACY_ZONE;
        if (auto&& pStorageIt = m_storage.find(storageTepe); pStorageIt != m_storage.end()) {
            if (auto&& pValueIt = pStorageIt->second.find(key); pValueIt != pStorageIt->second.end()) {
                pStorageIt->second.erase(pValueIt);
                return true;
            }
        }
        return false;
    }

    Storage::Value Storage::GetImpl(StorageType storageTepe, ValueType valueType, StringAtom key, const std::optional<Value>& def)
    {
        SR_TRACY_ZONE;

        if (auto&& pStorageIt = m_storage.find(storageTepe); pStorageIt != m_storage.end()) {
            if (auto&& pValueIt = pStorageIt->second.find(key); pValueIt != pStorageIt->second.end()) {
                if (pValueIt->second.type != valueType) {
                    SRHalt("Storage::GetImpl() : value type mismatch for key \"{}\"!", key);
                }
                else {
                    return pValueIt->second.value;
                }
            }
        }

        if (def) {
            return def.value();
        }

        SRHalt("Storage::GetImpl() : key \"{}\" not found in storage!", key);
        return {};
    }

    void Storage::SetImpl(StorageType storageTepe, ValueType valueType, StringAtom key, Value value)
    {
        SR_TRACY_ZONE;

        if (auto&& pIt = m_storage.find(storageTepe); pIt != m_storage.end()) {
            pIt->second[key] = { value, valueType };
        }
        else {
            m_storage[storageTepe] = { { key, { value, valueType } } };
        }
    }

    Storage::ValueHolder::ValueHolder() = default;

    Storage::ValueHolder::ValueHolder(Value val, ValueType type)
        : value(val)
        , type(type)
    { }

    Storage::ValueHolder::ValueHolder(const ValueHolder& other)
        : type(other.type)
        , value(other.value)
    {
        if (type == ValueType::String && other.value.s) {
            value.s = new std::string(*other.value.s);
        }
    }

    Storage::ValueHolder::ValueHolder(ValueHolder&& other) noexcept
        : type(other.type)
        , value(other.value)
    {
        other.type = ValueType::AnyType;
        other.value = {};
    }

    Storage::ValueHolder& Storage::ValueHolder::operator=(const Storage::ValueHolder& other) {
        if (this != &other) {
            if (type == ValueType::String && value.s) {
                delete value.s;
                value.s = nullptr;
            }

            type = other.type;
            value = other.value;

            if (type == ValueType::String && other.value.s) {
                value.s = new std::string(*other.value.s);
            }
        }
        return *this;
    }

    Storage::ValueHolder& Storage::ValueHolder::operator=(Storage::ValueHolder&& other) noexcept {
        if (this != &other) {
            if (type == ValueType::String && value.s) {
                delete value.s;
                value.s = nullptr;
            }

            type = other.type;
            value = other.value;

            other.type = ValueType::AnyType;
            other.value = {};
        }
        return *this;
    }

    Storage::ValueHolder::~ValueHolder() {
        if (type == ValueType::String && value.s) {
            delete value.s;
            value.s = nullptr;
        }
    }
}