#pragma once

#ifndef __OBJECT_POOL_HPP__
#define __OBJECT_POOL_HPP__

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

namespace opo
{
    namespace _aux
    {
        static constexpr size_t CRIRICAL_MASS = 64;

        inline size_t _Find_Last_One(unsigned long block_)
        {
            if (block_ == 0)
            {
                return sizeof(unsigned long) * CHAR_BIT;
            }
#if defined(__GNUC__) || defined(__clang__)
            return __builtin_ctzl(block_);
#elif defined(_MSC_VER)
            unsigned long index = 0;
            _BitScanForward(&index, block_);
            return index;
#else  // Other compilers
            size_t index = 0;
            while ((block_ & 1) == 0)
            {
                block_ >>= 1;
                ++count;
            }
            return index;
#endif // __GNUC__ || __clang__
        }

        template<size_t N>
        inline size_t _Find_First(const std::bitset<N> &bitset_)
        {
            if constexpr (N < CRIRICAL_MASS)
            {
                for (size_t i = 0; i < N; ++i)
                {
                    if (bitset_.test(i))
                    {
                        return i;
                    }
                }
                return N;
            }
            else
            {
                constexpr size_t block_size = sizeof(unsigned long) * CHAR_BIT;
                constexpr size_t num_blocks = (N + block_size - 1) / block_size;
                unsigned long blocks[num_blocks] = { 0 };
                memcpy(blocks, &bitset_, sizeof(blocks));
                for (size_t i = 0; i < num_blocks; ++i)
                {
                    if (blocks[i] != 0)
                    {
                        size_t pos = i * block_size + _Find_Last_One(blocks[i]);
                        return (pos < N) ? pos : N;
                    }
                }
                return N;
            }
        }
    } // namespace _aux

    static constexpr size_t MAX_CAPACITY = 1024;

    template<typename T>
    class ObjectPool
    {
    public:
        using value_type = T;
        using pointer = T *;
        using const_pointer = const T *;
        using reference = T &;
        using const_reference = const T &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using unique_arr_ptr_type = std::unique_ptr<T[]>;
        using mutex_type = std::mutex;
        using lock_type = std::lock_guard<mutex_type>;
        using unique_lock_type = std::unique_lock<mutex_type>;
        using shared_ptr_type = std::shared_ptr<T>;
        using cleaner_type = std::function<void(T *)>;
        using bitset_type = std::bitset<MAX_CAPACITY>;

    public:
        ObjectPool(
            size_t capacity, cleaner_type cleaner = [](T *ptr) { (void)ptr; })
            : capacity_(capacity)
            , cleaner_(cleaner)
        {
            for (size_type i = 0; i < capacity_; ++i)
            {
                bitset_.set(i, true);
            }
            try
            {
                data_ = unique_arr_ptr_type(new T[capacity_]);
            }
            catch (std::bad_alloc &e)
            {
                throw std::runtime_error("ObjectPool::ObjectPool: bad alloc, due to " + std::string(e.what()));
                capacity_ = 0;
                bitset_.reset();
            }
        }

        ObjectPool(const ObjectPool &) = delete;
        ObjectPool(ObjectPool &&) = delete;
        ObjectPool &operator=(const ObjectPool &) = delete;
        ObjectPool &operator=(ObjectPool &&) = delete;

        ~ObjectPool()
        {
            for (size_type i = 0; i < capacity_; ++i)
            {
                if (bitset_[i])
                {
                    cleaner_(&data_[i]);
                }
            }
        }

        shared_ptr_type get_shared_pointer()
        {
            return std::shared_ptr<value_type>(alloc(),
                                               [this](value_type *ptr) {
                if (ptr)
                {
                    cleaner_(ptr);
                    free(ptr);
                }
            });
        }

        size_type capacity() const noexcept
        {
            return capacity_;
        }

        size_type size() noexcept
        {
            std::lock_guard<mutex_type> lock(mutex_);
            return capacity_ - bitset_.count();
        }

        pointer alloc() noexcept
        {
            std::lock_guard<mutex_type> lock(mutex_);
#if defined(__GNUC__) || defined(__clang__)
            size_type index = bitset_._Find_first();
#else
            size_type index = _aux::_Find_First(bitset_);
#endif // __GNUC__ || __clang__
            if (index == bitset_.size())
            {
                return nullptr;
            }
            bitset_.set(index, false);
            return &data_[index];
        }

        void free(pointer ptr) noexcept
        {
            std::lock_guard<mutex_type> lock(mutex_);
            bitset_.set(ptr - data_.get(), true);
        }

    protected:
        size_type capacity_{ 0 };
        unique_arr_ptr_type data_{ nullptr };
        mutex_type mutex_;
        cleaner_type cleaner_;
        bitset_type bitset_;
    };

    template<typename T>
    inline std::shared_ptr<T> get_shared_pointer_from(ObjectPool<T> &pool)
    {
        auto ptr = pool.get_shared_pointer();
        if (!ptr)
        {
            ptr = std::make_shared<T>();
        }
        return ptr;
    }
} // namespace opo

#endif // __OBJECT_POOL_HPP__
