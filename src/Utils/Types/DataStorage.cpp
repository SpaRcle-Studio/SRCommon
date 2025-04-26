//
// Created by Monika on 26.04.2025.
//

#include <Utils/Types/DataStorage.h>

namespace SR_HTYPES_NS {
    DataStorage::~DataStorage() {
        Clear();
    }

    DataStorage::DataStorage(DataStorage&& data) noexcept {
        m_pointers = std::exchange(data.m_pointers, {});
        m_values = std::exchange(data.m_values, {});
    }

    DataStorage& DataStorage::operator=(DataStorage&& data) noexcept {
        m_pointers = std::exchange(data.m_pointers, {});
        m_values = std::exchange(data.m_values, {});
        return *this;
    }

    void DataStorage::Clear() {
        m_pointers.clear();
        m_values.clear();
    }

    DataStorage::DataStorage() {

    }
}