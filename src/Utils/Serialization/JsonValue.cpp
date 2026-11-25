//
// Created by Codex on 15.11.2026.
//

#include <Utils/Serialization/JsonValue.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Resources/ResourceManager.h>

#include <yyjson.h>

#include <Codegen/JsonValue.generated.hpp>

namespace SR_UTILS_NS {
    struct JsonValue::Impl {
        yyjson_mut_doc* m_pDocument = nullptr;
        yyjson_mut_val value = {};

        Impl() {
            yyjson_mut_set_null(&value);
        }

        Impl(const Impl& other) : Impl() {
            Copy(&other.value);
        }

        Impl(Impl&& other) noexcept
            : m_pDocument(std::exchange(other.m_pDocument, nullptr)), value(other.value)
        {
            yyjson_mut_set_null(&other.value);
        }

        ~Impl() {
            yyjson_mut_doc_free(m_pDocument);
        }

        Impl& operator=(const Impl& other) {
            Impl copy(other);
            return *this = std::move(copy);
        }

        Impl& operator=(Impl&& other) noexcept {
            if (this != &other) {
                Reset();
                m_pDocument = std::exchange(other.m_pDocument, nullptr);
                value = other.value;
                yyjson_mut_set_null(&other.value);
            }
            return *this;
        }

        void Reset() {
            yyjson_mut_doc_free(m_pDocument);
            m_pDocument = nullptr;
            value = {};
            yyjson_mut_set_null(&value);
        }

        yyjson_mut_doc* Document() {
            if (!m_pDocument) {
                m_pDocument = yyjson_mut_doc_new(nullptr);
                if (!m_pDocument) {
                    throw std::bad_alloc();
                }
            }
            return m_pDocument;
        }

        void Copy(const yyjson_mut_val* pValue) {
            if (yyjson_mut_is_str(pValue) && yyjson_mut_get_len(pValue) == 0) {
                yyjson_mut_set_str(&value, "");
                return;
            }
            if (yyjson_mut_is_ctn(pValue) && yyjson_mut_get_len(pValue) == 0) {
                value = *pValue;
                value.next = nullptr;
                return;
            }
            if (yyjson_mut_is_str(pValue) || yyjson_mut_is_ctn(pValue)) {
                auto&& pCopy = yyjson_mut_val_mut_copy(Document(), pValue);
                if (!pCopy) {
                    Reset();
                    throw std::bad_alloc();
                }
                value = *pCopy;
            }
            else {
                value = *pValue;
            }
            value.next = nullptr;
        }

        static bool ReplaceScalar(yyjson_mut_val* pTarget, const yyjson_mut_val* pSource) {
            if (!pTarget || yyjson_mut_is_ctn(pSource) || yyjson_mut_is_str(pSource)) {
                return false;
            }
            yyjson_mut_val* pNext = pTarget->next;
            *pTarget = *pSource;
            pTarget->next = pNext;
            return true;
        }

        static void* WriteAllocate(void* pContext, size_t size) {
            auto&& result = *static_cast<String*>(pContext);
            result.resize(size);
            return result.data();
        }

        static void* WriteReallocate(void* pContext, void* pMemory, size_t oldSize, size_t size) {
            return WriteAllocate(pContext, size);
        }

        static void WriteFree(void* pContext, void* pMemory) {
        }
    };

    JsonValue::Impl& JsonValue::Data() {
        return *std::launder(reinterpret_cast<Impl*>(m_storage));
    }

    const JsonValue::Impl& JsonValue::Data() const {
        return *std::launder(reinterpret_cast<const Impl*>(m_storage));
    }

    JsonValue::JsonValue() {
        static_assert(sizeof(Impl) <= sizeof(m_storage));
        static_assert(alignof(Impl) <= 8);
        new (m_storage) Impl();
    }

    JsonValue::JsonValue(const JsonValue& other)
        : Serializable(other)
        , m_valid(other.m_valid)
    {
        SR_TRACY_ZONE;
        new (m_storage) Impl(other.Data());
    }

    JsonValue::JsonValue(JsonValue&& other) noexcept
        : Serializable(std::move(other))
        , m_valid(other.m_valid)
    {
        SR_TRACY_ZONE;
        new (m_storage) Impl(std::move(other.Data()));
    }

    JsonValue::~JsonValue() {
        SR_TRACY_ZONE;
        Data().~Impl();
    }

    JsonValue& JsonValue::operator=(const JsonValue& other) {
        if (this != &other) {
            SR_TRACY_ZONE;
            Serializable::operator=(other);
            Data() = other.Data();
            m_valid = other.m_valid;
        }
        return *this;
    }

    JsonValue& JsonValue::operator=(JsonValue&& other) noexcept {
        if (this != &other) {
            SR_TRACY_ZONE;
            Serializable::operator=(std::move(other));
            Data() = std::move(other.Data());
            m_valid = other.m_valid;
        }
        return *this;
    }

    void JsonValue::Save(ISerializer& serializer) const {
        SR_TRACY_ZONE;
        serializer.WriteString(View().ToJson(), SerializationId::Create("json"));
    }

    bool JsonValue::Load(IDeserializer& deserializer) {
        SR_TRACY_ZONE;
        String json;
        deserializer.ReadString(json, SerializationId::Create("json"));
        return Parse(json);
    }

    Optional<JsonValue> JsonValue::ParseString(StringView data) {
        JsonValue json;
        if (json.Parse(data)) {
            return json;
        }
        return {};
    }

    bool JsonValue::Parse(StringView json) {
        SR_TRACY_ZONE;
        if (json.empty()) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::Parse() : empty JSON");
            return false;
        }
        yyjson_read_err error = {};
        auto&& pParsed = yyjson_read_opts(const_cast<char*>(json.data()), json.size(), YYJSON_READ_ALLOW_BOM, nullptr, &error);
        if (!pParsed) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::Parse() : invalid JSON at byte {}: {}", error.pos, error.msg);
            return false;
        }
        auto&& pRoot = yyjson_doc_get_root(pParsed);
        if (!yyjson_is_ctn(pRoot) && !yyjson_is_str(pRoot)) {
            SetNull();
            if (yyjson_is_bool(pRoot)) {
                yyjson_mut_set_bool(&Data().value, yyjson_get_bool(pRoot));
            }
            else if (yyjson_is_uint(pRoot)) {
                yyjson_mut_set_uint(&Data().value, yyjson_get_uint(pRoot));
            }
            else if (yyjson_is_sint(pRoot)) {
                yyjson_mut_set_sint(&Data().value, yyjson_get_sint(pRoot));
            }
            else if (yyjson_is_real(pRoot)) {
                yyjson_mut_set_real(&Data().value, yyjson_get_real(pRoot));
            }
            yyjson_doc_free(pParsed);
            return true;
        }
        auto&& pDocument = yyjson_doc_mut_copy(pParsed, nullptr);
        yyjson_doc_free(pParsed);
        if (!pDocument) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::Parse() : cannot allocate JSON document");
            return false;
        }
        Data().Reset();
        Data().m_pDocument = pDocument;
        Data().value = *yyjson_mut_doc_get_root(pDocument);
        m_valid = true;
        return true;
    }

    bool JsonValue::LoadResource(StringView path) {
        SR_TRACY_ZONE;
        const Path resourcePath(path);
        const auto resolved = ResourceManager::Instance().GetResPath().Concat(resourcePath);
        if (!resolved.IsFile()) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::LoadResource() : resource not found: {}", path);
            return false;
        }
        String text;
        if (!FileSystem::ReadFile(resolved, text)) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::LoadResource() : cannot read resource: {}", path);
            return false;
        }
        if (!Parse(text)) {
            SR_ERROR("JsonValue::LoadResource() : invalid JSON resource: {}", path);
            return false;
        }
        return true;
    }

    String JsonValueView::ToJson() const {
        SR_TRACY_ZONE;
        if (!IsValid()) {
            return {};
        }
        String result;
        const yyjson_alc allocator = { JsonValue::Impl::WriteAllocate, JsonValue::Impl::WriteReallocate, JsonValue::Impl::WriteFree, &result };
        size_t length = 0;
        yyjson_write_err error = {};
        const yyjson_write_flag flags = YYJSON_WRITE_PRETTY | YYJSON_WRITE_NEWLINE_AT_END;
        if (!yyjson_mut_val_write_opts(static_cast<yyjson_mut_val*>(m_value), flags, &allocator, &length, &error)) {
            SR_ERROR("JsonValueView::ToJson() : cannot write JSON: {}", error.msg);
            return {};
        }
        result.resize(length);
        return result;
    }

    JsonValue JsonValueView::Clone() const {
        SR_TRACY_ZONE;
        JsonValue result;
        result.m_valid = IsValid();
        if (IsValid()) {
            result.Data().Copy(static_cast<const yyjson_mut_val*>(m_value));
        }
        return result;
    }

    JsonValueKind JsonValueView::GetKind() const {
        switch (yyjson_mut_get_type(static_cast<yyjson_mut_val*>(m_value))) {
            case YYJSON_TYPE_BOOL: return JsonValueKind::Boolean;
            case YYJSON_TYPE_NUM: return yyjson_mut_is_real(static_cast<yyjson_mut_val*>(m_value)) ? JsonValueKind::Number : JsonValueKind::Integer;
            case YYJSON_TYPE_STR: return JsonValueKind::String;
            case YYJSON_TYPE_ARR: return JsonValueKind::Array;
            case YYJSON_TYPE_OBJ: return JsonValueKind::Object;
            default: return JsonValueKind::Null;
        }
    }

    bool JsonValueView::Expect(JsonValueKind kind) const {
        if (!IsValid() || GetKind() != kind) {
            SR_ERROR("JsonValueView::Expect() : expected {}, actual {}, valid={}", static_cast<uint8_t>(kind), static_cast<uint8_t>(GetKind()), IsValid());
            return false;
        }
        return true;
    }

    uint32_t JsonValueView::GetCount() const {
        return static_cast<uint32_t>(yyjson_mut_arr_size(static_cast<yyjson_mut_val*>(m_value)) + yyjson_mut_obj_size(static_cast<yyjson_mut_val*>(m_value)));
    }

    bool JsonValueView::Has(StringView key) const {
        if (!Expect(JsonValueKind::Object)) {
            return false;
        }
        return yyjson_mut_obj_getn(static_cast<yyjson_mut_val*>(m_value), key.empty() ? "" : key.data(), key.size()) != nullptr;
    }

    JsonValueView JsonValueView::Get(StringView key) const {
        if (Expect(JsonValueKind::Object)) {
            if (auto&& pValue = yyjson_mut_obj_getn(static_cast<yyjson_mut_val*>(m_value), key.empty() ? "" : key.data(), key.size())) {
                return JsonValueView(pValue);
            }
        }
        SR_ERROR("JsonValueView::Get() : missing member {}", key);
        return {};
    }

    JsonValueView JsonValueView::At(uint32_t index) const {
        if (Expect(JsonValueKind::Array)) {
            if (auto&& pValue = yyjson_mut_arr_get(static_cast<yyjson_mut_val*>(m_value), index)) {
                return JsonValueView(pValue);
            }
        }
        SR_ERROR("JsonValueView::At() : invalid array index {}", index);
        return {};
    }

    String JsonValueView::GetKey(uint32_t index) const {
        if (!Expect(JsonValueKind::Object) || index >= yyjson_mut_obj_size(static_cast<yyjson_mut_val*>(m_value))) {
            SR_ERROR("JsonValueView::GetKey() : invalid object index {}", index);
            return {};
        }
        auto iterator = yyjson_mut_obj_iter_with(static_cast<yyjson_mut_val*>(m_value));
        auto&& pKey = yyjson_mut_obj_iter_next(&iterator);
        for (uint32_t i = 0; i < index; ++i) {
            pKey = yyjson_mut_obj_iter_next(&iterator);
        }
        return String(StringView(yyjson_mut_get_str(pKey), yyjson_mut_get_len(pKey)));
    }

    bool JsonValue::Set(StringView key, const JsonValue& value) {
        if (!View().Expect(JsonValueKind::Object) || !value.m_valid) {
            return false;
        }
        auto&& pExisting = yyjson_mut_obj_getn(&Data().value, key.empty() ? "" : key.data(), key.size());
        if (Impl::ReplaceScalar(pExisting, &value.Data().value)) {
            return true;
        }
        auto&& pDocument = Data().Document();
        auto&& pCopy = yyjson_mut_val_mut_copy(pDocument, &value.Data().value);
        if (!pCopy) {
            return false;
        }
        yyjson_mut_val lookup = {};
        yyjson_mut_set_strn(&lookup, key.empty() ? "" : key.data(), key.size());
        if (yyjson_mut_obj_replace(&Data().value, &lookup, pCopy)) {
            return true;
        }
        auto&& pKey = yyjson_mut_strncpy(pDocument, key.empty() ? "" : key.data(), key.size());
        return pKey && yyjson_mut_obj_add(&Data().value, pKey, pCopy);
    }

    bool JsonValue::SetAt(uint32_t index, const JsonValue& value) {
        if (!View().Expect(JsonValueKind::Array) || !value.m_valid || index >= yyjson_mut_arr_size(&Data().value)) {
            SR_ERROR("JsonValue::SetAt() : invalid array index {}", index);
            return false;
        }
        if (Impl::ReplaceScalar(yyjson_mut_arr_get(&Data().value, index), &value.Data().value)) {
            return true;
        }
        auto&& pCopy = yyjson_mut_val_mut_copy(Data().Document(), &value.Data().value);
        return pCopy && yyjson_mut_arr_replace(&Data().value, index, pCopy);
    }

    bool JsonValue::Append(const JsonValue& value) {
        if (!View().Expect(JsonValueKind::Array) || !value.m_valid) {
            return false;
        }
        auto&& pCopy = yyjson_mut_val_mut_copy(Data().Document(), &value.Data().value);
        return pCopy && yyjson_mut_arr_append(&Data().value, pCopy);
    }

    void JsonValue::SetNull() {
        Data().Reset();
        m_valid = true;
    }

    void JsonValue::MakeObject() {
        SetNull();
        yyjson_mut_set_obj(&Data().value);
    }

    void JsonValue::MakeArray() {
        SetNull();
        yyjson_mut_set_arr(&Data().value);
    }

    void JsonValue::SetBool(bool value) {
        SetNull();
        yyjson_mut_set_bool(&Data().value, value);
    }

    void JsonValue::SetInt(int64_t value) {
        SetNull();
        yyjson_mut_set_sint(&Data().value, value);
    }

    void JsonValue::SetNumber(double_t value) {
        SetNull();
        if (!std::isfinite(value)) {
            m_valid = false;
            SR_ERROR("JsonValue::SetNumber() : non-finite number");
            return;
        }
        yyjson_mut_set_real(&Data().value, value);
    }

    void JsonValue::SetString(StringView value) {
        if (yyjson_mut_is_str(&Data().value) && value.size() <= yyjson_mut_get_len(&Data().value)) {
            auto&& pBuffer = const_cast<char*>(yyjson_mut_get_str(&Data().value));
            if (!value.empty()) {
                std::memmove(pBuffer, value.data(), value.size());
            }
            // Empty strings may point to a string literal.
            if (yyjson_mut_get_len(&Data().value) != 0) {
                pBuffer[value.size()] = '\0';
            }
            yyjson_mut_set_strn(&Data().value, pBuffer, value.size());
            m_valid = true;
            return;
        }
        SetNull();
        if (value.empty()) {
            yyjson_mut_set_str(&Data().value, "");
            return;
        }
        if (!yyjson_mut_set_strncpy(&Data().value, value.empty() ? "" : value.data(), value.size(), Data().Document())) {
            m_valid = false;
            SR_ERROR("JsonValue::SetString() : cannot allocate JSON string");
            return;
        }
        m_valid = true;
    }

    bool JsonValueView::GetBool() const {
        return Expect(JsonValueKind::Boolean) && yyjson_mut_get_bool(static_cast<yyjson_mut_val*>(m_value));
    }

    double_t JsonValueView::GetNumber() const {
        if (IsValid() && GetKind() == JsonValueKind::Integer) {
            return yyjson_mut_get_num(static_cast<yyjson_mut_val*>(m_value));
        }
        return Expect(JsonValueKind::Number) ? yyjson_mut_get_real(static_cast<yyjson_mut_val*>(m_value)) : 0;
    }

    JsonValueView JsonValue::View() const {
        return m_valid ? JsonValueView(const_cast<yyjson_mut_val*>(&Data().value)) : JsonValueView();
    }

    /// ============================================ JsonIterator ======================================================

    StringView JsonValueView::GetString() const noexcept {
        if (!yyjson_mut_is_str(static_cast<yyjson_mut_val*>(m_value))) {
            return {};
        }

        return { yyjson_mut_get_str(static_cast<yyjson_mut_val*>(m_value)), yyjson_mut_get_len(static_cast<yyjson_mut_val*>(m_value)) };
    }

    int64_t JsonValueView::GetInt() const {
        return Expect(JsonValueKind::Integer) ? yyjson_mut_get_sint(static_cast<yyjson_mut_val*>(m_value)) : 0;
    }

    bool JsonValueView::IsArray() const noexcept {
        return yyjson_mut_is_arr(static_cast<yyjson_mut_val*>(m_value));
    }

    bool JsonValueView::IsNull() const noexcept {
        return yyjson_mut_is_null(static_cast<yyjson_mut_val*>(m_value));
    }

    bool JsonValueView::Contains(StringView key) const noexcept {
        return yyjson_mut_obj_getn(static_cast<yyjson_mut_val*>(m_value), key.empty() ? "" : key.data(), key.size()) != nullptr;
    }

    JsonValueView JsonValueView::operator[](StringView key) const noexcept {
        return JsonValueView(yyjson_mut_obj_getn(static_cast<yyjson_mut_val*>(m_value), key.empty() ? "" : key.data(), key.size()));
    }

    JsonIterator JsonValueView::begin() const noexcept {
        return JsonIterator(m_value);
    }

    JsonIterator JsonValueView::end() const noexcept {
        return {};
    }

    JsonIterator::JsonIterator(const void* pValue) noexcept {
        auto&& pValueInternal = const_cast<yyjson_mut_val*>(static_cast<const yyjson_mut_val*>(pValue));
        m_isObject = yyjson_mut_is_obj(pValueInternal);

        if (m_isObject) {
            auto&& iter = yyjson_mut_obj_iter_with(pValueInternal);
            *std::launder(reinterpret_cast<yyjson_mut_obj_iter*>(m_storage)) = iter;
        }
        else {
            auto&& iter = yyjson_mut_arr_iter_with(pValueInternal);
            *std::launder(reinterpret_cast<yyjson_mut_arr_iter*>(m_storage)) = iter;
        }
        operator++();
    }

    JsonIterator& JsonIterator::operator++() noexcept {
        if (m_isObject) {
            m_key = yyjson_mut_obj_iter_next(std::launder(reinterpret_cast<yyjson_mut_obj_iter*>(m_storage)));
            m_current = m_key ? yyjson_mut_obj_iter_get_val(static_cast<yyjson_mut_val*>(m_key)) : nullptr;
        }
        else {
            m_current = yyjson_mut_arr_iter_next(std::launder(reinterpret_cast<yyjson_mut_arr_iter*>(m_storage)));
        }

        return *this;
    }

    JsonValueView JsonIterator::operator*() const noexcept {
        return JsonValueView(const_cast<void*>(m_current));
    }

    StringView JsonIterator::key() const noexcept {
        if (!m_key) {
            return {};
        }
        return { yyjson_mut_get_str(static_cast<yyjson_mut_val*>(m_key)), yyjson_mut_get_len(static_cast<yyjson_mut_val*>(m_key)) };
    }
}
