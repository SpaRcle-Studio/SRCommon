//
// Created by innerviewer on 10/26/2023.
//

#include <Utils/Common/Vertices.h>

#ifdef SR_COMMON_MESHOPTIMIZER
    #include <meshoptimizer.h>
#endif

#ifdef SR_UTILS_ASSIMP
    #include <assimp/scene.h>
#endif

#include <Enum/VertexAttributeFormat.hpp>
#include <Enum/VertexAttribute.hpp>

namespace SR_UTILS_NS {
    namespace Details {
        bool OptimizeVerticesImpl(
            const void* vertices,
            uint64_t vertexSize,
            uint64_t verticesCount,
            const SR_HTYPES_NS::FastMemoryArray<uint32_t>& indices,
            uint64_t targetIndicesCount,
            float_t targetError,
            SR_HTYPES_NS::FastMemoryArray<uint32_t>& outSimplifiedIndices
        ) {
            SR_TRACY_ZONE;

            outSimplifiedIndices.resize(indices.size());
            if (indices.empty()) {
                return true;
            }

        #ifdef SR_COMMON_MESHOPTIMIZER
            float resultError = 0.0f;
            uint32_t options = 0;

            size_t resultCount = meshopt_simplify(
                outSimplifiedIndices.data(),
                indices.data(),
                indices.size(),
                (float*)vertices,
                verticesCount,
                vertexSize,
                targetIndicesCount,
                targetError,
                options,
                &resultError
            );

            if (resultCount == 0) {
                SR_ERROR("OptimizeVerticesImpl() : mesh simplification failed!");
                outSimplifiedIndices.clear();
                return false;
            }

            outSimplifiedIndices.resize(resultCount);
            return true;
        #else
            memcpy(outSimplifiedIndices.data(), indices.data(), indices.size() * sizeof(uint32_t));
            return true;
        #endif
        }
    }

    uint8_t GetVertexAttributeFormatSize(VertexAttributeFormat format) {
        constexpr uint8_t count = static_cast<uint8_t>(VertexAttributeFormat::VertexAttributeFormatMAX);
        constexpr uint8_t sizes[count] = {
            0, // None
            4, // Float32
            2, // Float16
            4, // UInt32
            4, // Int32
            2, // UInt16
            2, // Int16
            1, // UInt8
            1, // Int8
            1, // UNorm8
            2, // UNorm16
            1, // SNorm8
            2, // SNorm16
            4, // R10G10B10A2_UNorm
            4, // R11G11B10_Float
        };

        const auto index = static_cast<uint8_t>(format);
        if (index >= count || sizes[index] == 0) {
            SRHalt("GetVertexAttributeFormatSize() : unknown vertex attribute format! Format: {}", format);
            return 0;
        }
        return sizes[index];
    }

    VertexAttribute GetVertexAttributeByIndex(VertexAttribute attribute, uint8_t index) {
        switch (attribute) {
            case VertexAttribute::UV0:
                return static_cast<VertexAttribute>(static_cast<uint64_t>(VertexAttribute::UV0) << index);
            case VertexAttribute::Color0:
                return static_cast<VertexAttribute>(static_cast<uint64_t>(VertexAttribute::Color0) << index);
            default:
                break;
        }

        SRHalt("GetVertexAttributeByIndex() : vertex attribute does not support indexing! Attribute: {}, Index: {}", attribute, index);
        return VertexAttribute::None;
    }

    std::string_view VertexAttributeToName(VertexAttribute attribute) {
        switch (attribute) {
            case VertexAttribute::Position:       return "VERTEX"sv;
            case VertexAttribute::Normal:         return "NORMAL"sv;
            case VertexAttribute::Tangent:        return "TANGENT"sv;
            case VertexAttribute::UV0:            return "UV"sv;
            case VertexAttribute::UV1:            return "UV1"sv;
            case VertexAttribute::UV2:            return "UV2"sv;
            case VertexAttribute::UV3:            return "UV3"sv;
            case VertexAttribute::UV4:            return "UV4"sv;
            case VertexAttribute::UV5:            return "UV5"sv;
            case VertexAttribute::UV6:            return "UV6"sv;
            case VertexAttribute::UV7:            return "UV7"sv;
            case VertexAttribute::Color0:         return "COLOR0"sv;
            case VertexAttribute::Color1:         return "COLOR1"sv;
            case VertexAttribute::Color2:         return "COLOR2"sv;
            case VertexAttribute::Color3:         return "COLOR3"sv;
            case VertexAttribute::Color4:         return "COLOR4"sv;
            case VertexAttribute::Color5:         return "COLOR5"sv;
            case VertexAttribute::Color6:         return "COLOR6"sv;
            case VertexAttribute::Color7:         return "COLOR7"sv;
            case VertexAttribute::BlendIndices:   return "BLEND_INDICES"sv;
            case VertexAttribute::BlendWeights:   return "BLEND_WEIGHTS"sv;
            case VertexAttribute::BlendIndices2:   return "BLEND_INDICES2"sv;
            case VertexAttribute::BlendWeights2:   return "BLEND_WEIGHTS2"sv;
            case VertexAttribute::MaterialIndices:  return "MATERIAL_INDICES"sv;
            case VertexAttribute::MaterialWeights:  return "MATERIAL_WEIGHTS"sv;
            case VertexAttribute::MaterialID0:      return "MATERIAL_ID0"sv;
            case VertexAttribute::MaterialID1:      return "MATERIAL_ID1"sv;
            case VertexAttribute::MaterialID2:      return "MATERIAL_ID2"sv;
            case VertexAttribute::MaterialID3:      return "MATERIAL_ID3"sv;
            case VertexAttribute::MaterialID4:      return "MATERIAL_ID4"sv;
            case VertexAttribute::MaterialID5:      return "MATERIAL_ID5"sv;
            case VertexAttribute::MaterialID6:      return "MATERIAL_ID6"sv;
            case VertexAttribute::MaterialID7:      return "MATERIAL_ID7"sv;
            case VertexAttribute::BlendFactor:      return "BLEND_FACTOR"sv;
            case VertexAttribute::Custom0:        return "CUSTOM0"sv;
            case VertexAttribute::Custom1:        return "CUSTOM1"sv;
            case VertexAttribute::Custom2:        return "CUSTOM2"sv;
            case VertexAttribute::Custom3:        return "CUSTOM3"sv;
            case VertexAttribute::Custom4:        return "CUSTOM4"sv;
            case VertexAttribute::Custom5:        return "CUSTOM5"sv;
            case VertexAttribute::Custom6:        return "CUSTOM6"sv;
            case VertexAttribute::Custom7:        return "CUSTOM7"sv;
            default:
                SRHalt("VertexAttributeToName() : unknown vertex attribute! Attribute: {}", attribute);
                return "Unknown"sv;
        }
    }

    uint8_t VertexAttributeDescription::GetAttributeSizeInBytes() const {
        return GetAttributeSizeInBytes(format, count);
    }

    uint8_t VertexAttributeDescription::GetAttributeSizeInBytes(VertexAttributeFormat format, uint8_t count) {
        // Для packed форматов count игнорируем, так как они уже 32-битные
        if (format == VertexAttributeFormat::R10G10B10A2_UNorm || format == VertexAttributeFormat::R11G11B10_Float) {
            return 4;
        }

        const uint8_t size = GetVertexAttributeFormatSize(format) * count;
        return size == 12 ? 16 : size; // GPU slot rule
    }

    uint64_t VertexLayoutDescription::GetStride() const {
        return stride;
    }

    const VertexAttributeDescription* VertexLayoutDescription::Find(VertexAttribute attribute) const {
        for (uint8_t i = 0; i < attributesCount; ++i) {
            if (attributes[i].attribute == attribute) {
                return &attributes[i];
            }
        }
        return nullptr;
    }

    uint32_t VertexLayoutDescription::Align(uint32_t v, uint32_t a) {
        return (v + a - 1) & ~(a - 1);
    }

    VertexLayoutDescription& VertexLayoutDescription::AddAttribute(VertexAttribute attribute, VertexAttributeFormat format, uint8_t count) {
        if (attributesCount >= SR_MAX_VERTEX_ATTRIBUTES) {
            SRHalt("VertexLayoutDescription::AddAttribute() : maximum vertex attributes count exceeded! Max count: {}", SR_MAX_VERTEX_ATTRIBUTES);
            return *this;
        }

        uint32_t offset = stride;

        // GPU-safe alignment
        offset = Align(offset, 16);
        SRAssert(offset < std::numeric_limits<uint16_t>::max());

        uint32_t size = VertexAttributeDescription::GetAttributeSizeInBytes(format, count);
        attributes[attributesCount++] = VertexAttributeDescription{ attribute, format, count, (uint16_t)offset };

        stride = offset + size;
        return *this;
    }

    bool VertexLayoutDescription::Compare(const VertexLayoutDescription& other) const {
        SR_TRACY_ZONE;
        if (attributesCount != other.attributesCount) {
            return false;
        }
        for (uint8_t i = 0; i < attributesCount; ++i) {
            if (attributes[i].attribute != other.attributes[i].attribute ||
                attributes[i].offset != other.attributes[i].offset ||
                attributes[i].format != other.attributes[i].format ||
                attributes[i].count != other.attributes[i].count)
            {
                return false;
            }
        }
        return true;
    }

    uint64_t VertexLayoutDescription::GetHash() const {
        SR_TRACY_ZONE;
        uint64_t hash = 0;
        for (uint8_t i = 0; i < attributesCount; ++i) {
            const auto& attribute = attributes[i];
            hash ^= std::hash<uint64_t>()(static_cast<uint64_t>(attribute.attribute)) +
                    std::hash<uint64_t>()(static_cast<uint64_t>(attribute.format)) +
                    std::hash<uint64_t>()(attribute.count) +
                    std::hash<uint64_t>()(attribute.offset) +
                    0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }

    VertexDataBuffer::VertexDataBuffer(VertexDataBuffer&& other) noexcept {
        vertexCount = other.vertexCount;
        layout = other.layout;
        data = std::move(other.data);
    }

    VertexDataBuffer& VertexDataBuffer::operator=(VertexDataBuffer&& other) noexcept {
        if (this != &other) {
            vertexCount = other.vertexCount;
            layout = other.layout;
            data = std::move(other.data);
        }
        return *this;
    }

    void VertexDataBuffer::Allocate(uint64_t vertices) {
        SR_TRACY_ZONE;
        vertexCount = vertices;
        data.resize(vertices * layout.GetStride());
    }

    uint8_t* VertexDataBuffer::GetAttributeData(VertexAttribute attribute) {
        if (auto&& pDescription = layout.Find(attribute)) {
            return data.data() + pDescription->offset;
        }
        return nullptr;
    }

    void VertexDataBuffer::SetVertex(uint64_t index, VertexAttribute attribute, const void* pSrc) {
        if (auto&& pDescription = layout.Find(attribute)) {
            uint8_t* pDst = data.data() + pDescription->offset + index * layout.GetStride();
            std::memcpy(pDst, pSrc, pDescription->GetAttributeSizeInBytes());
        }
    }

    VertexDataBuffer VertexDataBuffer::TransitionToLayout(const VertexLayoutDescription& newLayout) const {
        SR_TRACY_ZONE;

        VertexDataBuffer newBuffer;
        newBuffer.layout = newLayout;
        newBuffer.Allocate(vertexCount);

        auto&& range = std::views::iota(0, static_cast<int>(vertexCount));
        SR_UTILS_NS::ForEach<SR_UTILS_NS::ExecutionPolicy::ParUnSeq>(range.begin(), range.end(), [&](uint64_t i) {
            for (uint64_t j = 0; j < newLayout.attributesCount; ++j) {
                const VertexAttributeDescription& newAttribute = newLayout.attributes[j];
                if (auto&& pOldDescription = layout.Find(newAttribute.attribute)) {
                    const uint8_t* pSrc = data.data() + pOldDescription->offset + i * layout.GetStride();
                    uint8_t* pDst = newBuffer.data.data() + newAttribute.offset + i * newLayout.GetStride();
                    std::memcpy(pDst, pSrc, newAttribute.GetAttributeSizeInBytes());
                }
            }
        });

        return newBuffer;
    }

    void VertexDataBuffer::CopyFrom(const VertexDataBuffer& other) {
        layout = other.layout;
        vertexCount = other.vertexCount;
        data.resize(other.data.size());
        std::memcpy(data.data(), other.data.data(), data.size());
    }

#ifdef SR_UTILS_ASSIMP
    void LoadBonesFromAssimp(const aiMesh* pMesh, const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>& bones, VertexDataBuffer& buffer) {
        SR_TRACY_ZONE;
        SR_GLOBAL_LOCK;

        constexpr uint32_t maxWeightsPerVertex = 16;
        struct BoneWeight {
            uint32_t boneId = SR_UINT32_MAX;
            float weight = 0.f;
        };
        struct BoneWeights {
            uint8_t count = 0;
            BoneWeight weights[maxWeightsPerVertex] = {};
        };

        // 1. собираем все веса по вершинам
        static SR_HTYPES_NS::FastMemoryArray<BoneWeights> vertexWeights;
        if (pMesh->mNumVertices > vertexWeights.capacity()) {
            if (vertexWeights.empty()) {
                vertexWeights.reserve(SR_MIN(pMesh->mNumVertices, 65536 * 16));
            }
            else {
                vertexWeights.reserve(SR_MAX(vertexWeights.size() * 2, pMesh->mNumVertices * 2));
            }
        }
        vertexWeights.resize(pMesh->mNumVertices);
        std::memset(vertexWeights.data(), 0, vertexWeights.size() * sizeof(BoneWeights));

        for (uint32_t i = 0; i < pMesh->mNumBones; ++i) {
            auto&& bone = pMesh->mBones[i];
            const uint32_t boneId = bones.at(SR_UTILS_NS::StringAtom(bone->mName.C_Str()));

            for (uint32_t j = 0; j < bone->mNumWeights; ++j) {
                const uint32_t vertexId = bone->mWeights[j].mVertexId;
                const float weightValue = bone->mWeights[j].mWeight;

                if (weightValue > 0.0f) {
                    if (vertexWeights[vertexId].count >= maxWeightsPerVertex) {
                        SR_WARN("Vertex {} has more than {} bone weights! Some weights will be omitted!", vertexId, SR_MAX_BONES_ON_VERTEX);
                        continue;
                    }
                    vertexWeights[vertexId].weights[vertexWeights[vertexId].count++] = { boneId, weightValue };
                }
            }
        }

        // 2. обрабатываем каждую вершину
        auto&& range = std::views::iota(0, static_cast<int>(pMesh->mNumVertices));
        SR_UTILS_NS::ForEach<SR_UTILS_NS::ExecutionPolicy::ParUnSeq>(range.begin(), range.end(), [&](uint64_t v) {
            auto& weights = vertexWeights[v];

            if (weights.count == 0) {
                // fallback — чтобы не улетело в (0,0,0)
                auto* pIdx = static_cast<SR_MATH_NS::UVector4*>(buffer.GetVertex(v, VertexAttribute::BlendIndices));
                auto* pW   = static_cast<SR_MATH_NS::FVector4*>(buffer.GetVertex(v, VertexAttribute::BlendWeights));

                (*pIdx)[0] = 0;
                (*pW)[0]   = 1.0f;
                return;
            }

            // 3. сортировка по убыванию веса
            std::stable_sort(weights.weights, weights.weights + weights.count, [](const BoneWeight& a, const BoneWeight& b) {
                return a.weight > b.weight;
            });

            // 4.  выбрать максимум влияний
            const uint32_t maxInfluences = std::min<uint32_t>(8, weights.count);

            // 5. сумма ТОЛЬКО выбранных
            float selectedSum = 0.0f;
            for (uint32_t i = 0; i < maxInfluences; ++i)
                selectedSum += weights.weights[i].weight;

            if (selectedSum <= 0.0f) {
                SRHalt("Selected weights sum is zero!");
                return;
            }

            // 6. нормализация выбранных
            const float invSum = 1.0f / selectedSum;

            SR_MATH_NS::UVector4 idx1 = {0,0,0,0};
            SR_MATH_NS::FVector4 w1   = {0,0,0,0};

            SR_MATH_NS::UVector4 idx2 = {0,0,0,0};
            SR_MATH_NS::FVector4 w2   = {0,0,0,0};

            uint32_t used = std::min<uint32_t>(maxInfluences, 8);

            for (uint32_t i = 0; i < used; ++i) {
                const float w = weights.weights[i].weight * invSum;

                if (i < 4) {
                    idx1[i] = weights.weights[i].boneId;
                    w1[i]   = w;
                }
                else {
                    idx2[i - 4] = weights.weights[i].boneId;
                    w2[i - 4]   = w;
                }
            }

            // 7. запись
            auto* pIdx1 = static_cast<SR_MATH_NS::UVector4*>(buffer.GetVertex(v, VertexAttribute::BlendIndices));
            auto* pW1   = static_cast<SR_MATH_NS::FVector4*>(buffer.GetVertex(v, VertexAttribute::BlendWeights));
            auto* pIdx2 = static_cast<SR_MATH_NS::UVector4*>(buffer.GetVertex(v, VertexAttribute::BlendIndices2));
            auto* pW2Ptr= static_cast<SR_MATH_NS::FVector4*>(buffer.GetVertex(v, VertexAttribute::BlendWeights2));

            *pIdx1 = idx1;
            *pW1   = w1;
            *pIdx2 = idx2;
            *pW2Ptr= w2;

            // 8. проверка
            float finalSum = 0.0f;
            for (uint32_t i = 0; i < maxInfluences; ++i) {
                finalSum += (i < 4 ? w1[i] : w2[i - 4]);
            }

            if (std::abs(finalSum - 1.0f) > 0.01f) {
                SR_ERROR("Bone weights for vertex {} do not sum to 1! Final sum: {}", v, finalSum);
            }
        });
    }

    VertexDataBuffer VertexDataBuffer::AllocateFromAssimp(aiMesh* pMesh, const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>& bones) {
        SR_TRACY_ZONE;

        VertexDataBuffer buffer;

        {
            SR_TRACY_ZONE_N("Setup vertex layout");

            buffer.layout.AddAttribute(VertexAttribute::Position, VertexAttributeFormat::Float32, 3);

            if (pMesh->HasNormals()) {
                buffer.layout.AddAttribute(VertexAttribute::Normal, VertexAttributeFormat::Float32, 3);
            }

            if (pMesh->HasTangentsAndBitangents()) {
                buffer.layout.AddAttribute(VertexAttribute::Tangent, VertexAttributeFormat::Float32, 4);
            }

            for (uint8_t i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i) {
                if (pMesh->HasTextureCoords(i)) {
                    buffer.layout.AddAttribute(GetVertexAttributeByIndex(VertexAttribute::UV0, i), VertexAttributeFormat::Float32, 2);
                }
            }

            for (uint8_t i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i) {
                if (pMesh->HasVertexColors(i)) {
                    buffer.layout.AddAttribute(GetVertexAttributeByIndex(VertexAttribute::Color0, i), VertexAttributeFormat::Float32, 4);
                }
            }

            uint32_t maxWeightsNum = 0;
            for (uint32_t i = 0; i < pMesh->mNumBones; i++) {
                maxWeightsNum = SR_MAX(maxWeightsNum, pMesh->mBones[i]->mNumWeights);
            }

            if (maxWeightsNum > 0) {
                buffer.layout.AddAttribute(VertexAttribute::BlendIndices, VertexAttributeFormat::UInt32, 4);
                buffer.layout.AddAttribute(VertexAttribute::BlendWeights, VertexAttributeFormat::Float32, 4);
                if (maxWeightsNum > 4) {
                    buffer.layout.AddAttribute(VertexAttribute::BlendIndices2, VertexAttributeFormat::UInt32, 4);
                    buffer.layout.AddAttribute(VertexAttribute::BlendWeights2, VertexAttributeFormat::Float32, 4);
                }
            }
        }

        buffer.Allocate(pMesh->mNumVertices);

        auto&& range = std::views::iota(0, static_cast<int>(pMesh->mNumVertices));
        SR_UTILS_NS::ForEach<SR_UTILS_NS::ExecutionPolicy::ParUnSeq>(range.begin(), range.end(), [&](uint64_t i) {
            buffer.SetVertex(i, VertexAttribute::Position, &pMesh->mVertices[i]);

            for (uint8_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j) {
                if (pMesh->HasTextureCoords(j)) {
                    buffer.SetVertex(i, GetVertexAttributeByIndex(VertexAttribute::UV0, j), &pMesh->mTextureCoords[j][i]);
                }
            }

            for (uint8_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j) {
                if (pMesh->HasVertexColors(j)) {
                    buffer.SetVertex(i, GetVertexAttributeByIndex(VertexAttribute::Color0, j), &pMesh->mColors[j][i]);
                }
            }

            if (pMesh->HasNormals()) {
                buffer.SetVertex(i, VertexAttribute::Normal, &pMesh->mNormals[i]);
            }

            if (pMesh->HasTangentsAndBitangents()) {
                buffer.SetVertex(i, VertexAttribute::Tangent, &pMesh->mTangents[i]);
            }
        });

        if (pMesh->mNumBones > 0) {
            LoadBonesFromAssimp(pMesh, bones, buffer);

            /*SR_TRACY_ZONE_N("Setup vertex bone weights");
            SR_HTYPES_NS::FastMemoryArray<uint8_t> weightsNumPerVertex;
            weightsNumPerVertex.resize(pMesh->mNumVertices);
            weightsNumPerVertex.fill(0);

            for (uint32_t i = 0; i < pMesh->mNumBones; i++) {
                auto&& bone = pMesh->mBones[i];
                const uint32_t boneId = bones.at(SR_UTILS_NS::StringAtom(bone->mName.C_Str()));
                for (uint32_t j = 0; j < bone->mNumWeights; j++) {
                    const uint32_t vertexId = bone->mWeights[j].mVertexId;
                    const float weightValue = bone->mWeights[j].mWeight;
                    uint8_t& vertexWeightsNum = weightsNumPerVertex[vertexId];

                    if (vertexWeightsNum < 4) {
                        SR_MATH_NS::UVector4* pBoneId = reinterpret_cast<SR_MATH_NS::UVector4*>(buffer.GetVertex(vertexId, VertexAttribute::BlendIndices));
                        SR_MATH_NS::FVector4* pWeight = reinterpret_cast<SR_MATH_NS::FVector4*>(buffer.GetVertex(vertexId, VertexAttribute::BlendWeights));
                        (*pBoneId)[vertexWeightsNum] = boneId;
                        (*pWeight)[vertexWeightsNum] = weightValue;
                    }
                    else if (vertexWeightsNum < 8) {
                        SR_MATH_NS::UVector4* pBoneId = reinterpret_cast<SR_MATH_NS::UVector4*>(buffer.GetVertex(vertexId, VertexAttribute::BlendIndices2));
                        SR_MATH_NS::FVector4* pWeight = reinterpret_cast<SR_MATH_NS::FVector4*>(buffer.GetVertex(vertexId, VertexAttribute::BlendWeights2));
                        (*pBoneId)[vertexWeightsNum - 4] = boneId;
                        (*pWeight)[vertexWeightsNum - 4] = weightValue;
                    }

                    vertexWeightsNum++;
                }
            }*/
        }

        return buffer;
    }

    void* VertexDataBuffer::GetVertex(uint64_t index, VertexAttribute attribute) {
        if (auto&& pDescription = layout.Find(attribute)) {
            return data.data() + pDescription->offset + index * layout.GetStride();
        }
        return nullptr;
    }

#endif
}
