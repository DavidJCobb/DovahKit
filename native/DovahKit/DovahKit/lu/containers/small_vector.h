#pragma once
#include <array>
#include <iterator> // std::reverse_iterator, std::make_reverse_iterator
#include <memory>
#include "../allocation_policies/exponential.h"
#include "../iterators/pointer_based_iterator.h"

namespace lu {
   template<
      typename T,
      size_t LocalCapacity,
      typename AllocationPolicy = allocation_policies::exponential,
      typename Allocator = std::allocator<T>
   >
   class small_vector {
      public:
         using allocation_policy = AllocationPolicy;
         using allocator_type    = Allocator;
         using value_type        = T;
         using size_type         = size_t;

         using iterator               = iterators::pointer_based_iterator<small_vector, value_type, false>;
         using const_iterator         = iterators::pointer_based_iterator<small_vector, value_type, true>;
         using reverse_iterator       = std::reverse_iterator<iterator>;
         using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      protected:
         static constexpr const size_type _heap_bit_index = sizeof(size_type) * 8 - 1;

      public:
         static constexpr const size_type local_capacity = LocalCapacity;
         static constexpr const size_type max_capacity   = ((size_type)1 << _heap_bit_index) - 1;

      protected:
         constexpr bool _is_storage_local() const noexcept;
         constexpr void _set_storage_is_heap() noexcept;
         constexpr void _set_storage_is_local() noexcept;
         constexpr void _set_size(size_type);

         constexpr void _constant_evaluation_init_local();

         constexpr void _move_storage_to_local();
         constexpr void _move_storage_to_local(size_t first_n, size_t skip_next);
         constexpr void _resize_without_initializing(size_type s);
         constexpr void _set_capacity_to(size_type capacity);
         constexpr void _transfer_storage(value_type* src, value_type* dst, size_type count);

      protected:
         size_t _size_and_flags = 0;
         union {
            struct {
               size_t      capacity;
               value_type* data;
            } _allocated;
            std::array<value_type, local_capacity> _local;

            // avoid default-constructing anything:
            bool _uninitialized = false;
         };
         [[maybe_unused]] allocator_type _allocator;

      public:
         constexpr small_vector() {}
         constexpr ~small_vector();

         constexpr size_type capacity() const noexcept;
         constexpr const value_type* data() const noexcept;
         constexpr value_type* data() noexcept;
         constexpr bool empty() const noexcept;
         constexpr size_type max_size() const noexcept { return max_capacity; }
         constexpr size_type size() const noexcept;

         #pragma region Element access
            constexpr const value_type& at(size_type) const;
            constexpr value_type& at(size_type);
            constexpr const value_type& back() const;
            constexpr value_type& back();
            constexpr const value_type& front() const;
            constexpr value_type& front();
            constexpr const value_type& operator[](size_type) const noexcept;
            constexpr value_type& operator[](size_type) noexcept;
         #pragma endregion

         #pragma region Reallocating
            constexpr void reserve(size_type);
            constexpr void resize(size_type);
            constexpr void resize(size_type, const value_type&);
            constexpr void shrink_to_fit();
         #pragma endregion

         #pragma region Modification
            constexpr void clear();
            constexpr void erase(const_iterator);
            constexpr void erase(const_iterator start, const_iterator end);
            constexpr void insert(const_iterator, const value_type&);
            constexpr void insert(const_iterator, value_type&&);
            constexpr void insert(size_type at, const value_type&);
            constexpr void insert(size_type at, value_type&&);
            constexpr void push_back(const value_type&);
            constexpr void push_back(value_type&&);
            constexpr void pop_back();

            template<typename... Args>
            constexpr value_type& emplace(const_iterator, Args&&...);
            template<typename... Args>
            constexpr value_type& emplace(size_type at, Args&&...);

            template<typename... Args>
            constexpr value_type& emplace_back(Args&&...);
         #pragma endregion

         #pragma region Operators
            constexpr bool operator==(const small_vector&) const noexcept;
            constexpr bool operator!=(const small_vector&) const noexcept;
         #pragma endregion

         #pragma region Iterators
            constexpr const_iterator cbegin() const noexcept;
            constexpr const_iterator begin() const noexcept { return cbegin(); }
            constexpr iterator begin() noexcept;
            constexpr const_iterator cend() const noexcept;
            constexpr const_iterator end() const noexcept { return cend(); }
            constexpr iterator end() noexcept;

            constexpr const_reverse_iterator crbegin() const;
            constexpr const_reverse_iterator rbegin() const { return crbegin(); };
            constexpr reverse_iterator rbegin();
            constexpr const_reverse_iterator crend() const;
            constexpr const_reverse_iterator rend() const { return crend(); };
            constexpr reverse_iterator rend();
         #pragma endregion
   };
}

#include "./small_vector.inl"