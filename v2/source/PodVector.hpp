// File adapted from dear-imgui's ImVector, implemented in https://github.com/ocornut/imgUI/blob/master/imgui.h
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <span>

template <class T>
class PodVector {
public:
    using value_type = T;
    using iterator = value_type*;
    using const_iterator = const value_type*;

private:
    int mSize;
    int mCapacity;
    T* mData;

public:
    PodVector() {
        mSize = mCapacity = 0;
        mData = nullptr;
    }

    PodVector(const PodVector<T>& src) {
        mSize = mCapacity = 0;
        mData = nullptr;
        operator=(src);
    }

    PodVector<T>& operator=(const PodVector<T>& src) {
        clear();
        resize(src.mSize);
        std::memcpy(mData, src.mData, (size_t)mSize * sizeof(T));
        return *this;
    }

    PodVector(PodVector&& src) {
        mSize = src.mSize;
        mCapacity = src.mCapacity;
        mData = src.mData;

        src.mSize = src.mCapacity = 0;
        src.mData = nullptr;
    }

    PodVector& operator=(PodVector&& src) {
        if (this != &src) {
            std::free(mData);

            mSize = src.mSize;
            mCapacity = src.mCapacity;
            mData = src.mData;

            src.mSize = src.mCapacity = 0;
            src.mData = nullptr;
        }
        return *this;
    }

    ~PodVector() {
        std::free(mData);
    }

    bool empty() const { return mSize == 0; }
    int size() const { return mSize; }
    int size_in_bytes() const { return mSize * (int)sizeof(T); }
    int max_size() const { return 0x7FFFFFFF / (int)sizeof(T); }
    int capacity() const { return mCapacity; }

    T& operator[](int i) {
        assert(i >= 0 && i < mSize);
        return mData[i];
    }

    const T& operator[](int i) const {
        assert(i >= 0 && i < mSize);
        return mData[i];
    }

    void clear() {
        if (mData) {
            mSize = mCapacity = 0;
            std::free(mData);
            mData = nullptr;
        }
    }

    T* begin() { return mData; }
    const T* begin() const { return mData; }
    T* end() { return mData + mSize; }
    const T* end() const { return mData + mSize; }

    T& front() {
        assert(mSize > 0);
        return mData[0];
    }

    const T& front() const {
        assert(mSize > 0);
        return mData[0];
    }

    T& back() {
        assert(mSize > 0);
        return mData[mSize - 1];
    }

    const T& back() const {
        assert(mSize > 0);
        return mData[mSize - 1];
    }

    void swap(PodVector<T>& rhs) {
        int rhs_size = rhs.mSize;
        rhs.mSize = mSize;
        mSize = rhs_size;
        int rhs_cap = rhs.mCapacity;
        rhs.mCapacity = mCapacity;
        mCapacity = rhs_cap;
        T* rhs_mDataTmp = rhs.mData;
        rhs.mData = mData;
        mData = rhs_mDataTmp;
    }

    int grow_capacity(int sz) const {
        int newCapacity = mCapacity ? (mCapacity + mCapacity / 2) : 8;
        return newCapacity > sz ? newCapacity : sz;
    }

    void resize(int new_size) {
        if (new_size > mCapacity) reserve(grow_capacity(new_size));
        mSize = new_size;
    }

    void resize_more(int size) {
        resize(mSize + size);
    }

    void resize(int new_size, const T& v) {
        if (new_size > mCapacity) reserve(grow_capacity(new_size));
        if (new_size > mSize) {
            for (int n = mSize; n < new_size; n++) {
                std::memcpy(&mData[n], &v, sizeof(v));
            }
        }
        mSize = new_size;
    }

    void resize_more(int size, const T& v) {
        resize(mSize + size, v);
    }

    void shrink(int new_size) {
        assert(new_size <= mSize);
        mSize = new_size;
    }

    /// Resize a vector to a smaller mSize, guaranteed not to cause a reallocation
    void reserve(int newCapacity) {
        if (newCapacity <= mCapacity) return;
        auto tmp = (T*)std::malloc((size_t)newCapacity * sizeof(T));
        if (mData) {
            std::memcpy(tmp, mData, (size_t)mSize * sizeof(T));
            std::free(mData);
        }
        mData = tmp;
        mCapacity = newCapacity;
    }

    void reserve_more(int size) {
        reserve(mSize + size);
    }

    /// NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the PodVector data itself! e.g. v.push_back(v[10]) is forbidden.
    void push_back(const T& v) {
        if (mSize == mCapacity) reserve(grow_capacity(mSize + 1));
        std::memcpy(&mData[mSize], &v, sizeof(v));
        mSize++;
    }

    void pop_back() {
        assert(mSize > 0);
        mSize--;
    }

    void push_front(const T& v) {
        if (mSize == 0) {
            push_back(v);
        } else {
            insert(mData, v);
        }
    }

    T* erase(const T* it) {
        assert(it >= mData && it < mData + mSize);
        const ptrdiff_t off = it - mData;
        std::memmove(mData + off, mData + off + 1, ((size_t)mSize - (size_t)off - 1) * sizeof(T));
        mSize--;
        return mData + off;
    }

    T* erase(const T* it, const T* it_last) {
        assert(it >= mData && it < mData + mSize && it_last > it && it_last <= mData + mSize);
        const ptrdiff_t count = it_last - it;
        const ptrdiff_t off = it - mData;
        std::memmove(mData + off, mData + off + count, ((size_t)mSize - (size_t)off - count) * sizeof(T));
        mSize -= (int)count;
        return mData + off;
    }

    T* erase_unsorted(const T* it) {
        assert(it >= mData && it < mData + mSize);
        const ptrdiff_t off = it - mData;
        if (it < mData + mSize - 1) std::memcpy(mData + off, mData + mSize - 1, sizeof(T));
        mSize--;
        return mData + off;
    }

    T* insert(const T* it, const T& v) {
        assert(it >= mData && it <= mData + mSize);
        const ptrdiff_t off = it - mData;
        if (mSize == mCapacity) reserve(grow_capacity(mSize + 1));
        if (off < (int)mSize) std::memmove(mData + off + 1, mData + off, ((size_t)mSize - (size_t)off) * sizeof(T));
        std::memcpy(&mData[off], &v, sizeof(v));
        mSize++;
        return mData + off;
    }

    bool contains(const T& v) const {
        const T* data = mData;
        const T* dataEnd = mData + mSize;
        while (data < dataEnd) {
            if (*data++ == v) return true;
        }
        return false;
    }

    T* find(const T& v) {
        T* data = mData;
        const T* dataEnd = mData + mSize;
        while (data < dataEnd)
            if (*data == v)
                break;
            else
                ++data;
        return data;
    }

    const T* find(const T& v) const {
        const T* data = mData;
        const T* dataEnd = mData + mSize;
        while (data < dataEnd)
            if (*data == v)
                break;
            else
                ++data;
        return data;
    }

    bool find_erase(const T& v) {
        const T* it = find(v);
        if (it < mData + mSize) {
            erase(it);
            return true;
        }
        return false;
    }

    bool find_erase_unsorted(const T& v) {
        const T* it = find(v);
        if (it < mData + mSize) {
            erase_unsorted(it);
            return true;
        }
        return false;
    }

    int index_from_ptr(const T* it) const {
        assert(it >= mData && it < mData + mSize);
        const ptrdiff_t off = it - mData;
        return (int)off;
    }

    // Custom utility functions

    std::span<T> as_span() { return { mData, (size_t)mSize }; }
    std::span<uint8_t> as_data_span() { return { (uint8_t*)mData, (size_t)mSize * sizeof(T) }; }
    std::span<const T> as_span() const { return { mData, (size_t)mSize }; }
    std::span<const uint8_t> as_data_span() const { return { (uint8_t*)mData, (size_t)mSize * sizeof(T) }; }
};
