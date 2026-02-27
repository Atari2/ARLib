#pragma once
#include "Threading.hpp"
namespace ARLib {
template <typename DataType>
class SyncData;
template <typename DataType>
class LockSyncData {
    friend SyncData<DataType>;
    DataType& m_data;
    UniqueLock<Mutex> m_lock;
    LockSyncData(SyncData<DataType>& data) : m_data{ data.m_data }, m_lock{ data.m_mutex } {}
    public:
    const DataType& data() const { return m_data; }
    DataType& data() { return m_data; }
    const DataType& operator*() const { return m_data; }
    DataType& operator*() { return m_data; }
    const DataType* operator->() const { return &m_data; }
    DataType* operator->() { return &m_data; }
};
template <typename DataType>
class SyncData {
    DataType m_data;
    Mutex m_mutex;
    friend LockSyncData<DataType>;
    public:
    SyncData(DataType type) : m_data{ move(type) }, m_mutex{} {}
    LockSyncData<DataType> lock() { return LockSyncData{ *this }; }
    template <typename Functor>
    requires CallableWith<Functor, AddLvalueReferenceT<DataType>>
    auto with_lock(Functor&& func) {
        UniqueLock l{ m_mutex };
        return func(m_data);
    }
    template <typename Functor>
    requires CallableWith<Functor, AddLvalueReferenceT<AddConstT<DataType>>>
    auto with_lock(Functor&& func) const {
        UniqueLock l{ m_mutex };
        return func(m_data);
    }
    DataType extract() { 
        DataType tp{ move(m_data) };
        return tp; 
    }
};
}    // namespace ARLib