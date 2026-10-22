#pragma once
#include "./small_vector.h"
#include <cassert>
#include <cstring> // std::memmove
#include <stdexcept> // std::out_of_range
#include <utility> // std::as_const
#include <version>

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T, size_t LocalCapacity, typename AllocationStrategy, typename Allocator>
#define CLASS_NAME small_vector<T, LocalCapacity, AllocationStrategy, Allocator>

namespace lu {
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::~small_vector() {
      const size_type size = this->size();
      if (size != 0) {
         value_type* data = this->data();
         for (size_type i = 0; i < size; ++i) {
            std::destroy_at(data + i);
         }
      }
      if (!_is_storage_local()) {
         this->_allocator.deallocate(this->_allocated.data, this->_allocated.capacity);
         this->_local = {};
         this->_set_storage_is_local();
      }
      this->_size_and_flags = 0;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::_is_storage_local() const noexcept {
      return !((this->_size_and_flags >> _heap_bit_index) & 1);
   }
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_set_storage_is_heap() noexcept {
      this->_size_and_flags |= ((size_type)1 << _heap_bit_index);
   }
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_set_storage_is_local() noexcept {
      this->_size_and_flags &= ~((size_type)1 << _heap_bit_index);
   }
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_set_size(size_type s) {
      this->_size_and_flags &= ((size_type)1 << _heap_bit_index);
      this->_size_and_flags |= s;
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_constant_evaluation_init_local() {
      if (std::is_constant_evaluated()) {
         if (this->_size_and_flags == 0) // size 0 + no "heap" flag
            this->_local = {};
      }
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_move_storage_to_local() {
      if (std::is_constant_evaluated()) {
         this->_constant_evaluation_init_local();
      }
      const size_type prior_capacity = this->_allocated.capacity;
      value_type*     prior_data     = this->_allocated.data;
      if (std::is_constant_evaluated()) {
         this->_local = {}; // lifetime
      }
      if (prior_data) {
         try {
            this->_transfer_storage(prior_data, this->_local.data(), this->size());
         } catch (...) {
            this->_allocated = {
               .capacity = prior_capacity,
               .data     = prior_data,
            };
            throw;
         }
         this->_allocator.deallocate(prior_data, prior_capacity);
      }
      this->_set_storage_is_local();
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_move_storage_to_local(size_t first_n, size_t skip_next) {
      if (std::is_constant_evaluated()) {
         this->_constant_evaluation_init_local();
      }
      const size_type prior_capacity = this->_allocated.capacity;
      value_type*     prior_data     = this->_allocated.data;
      if (std::is_constant_evaluated()) {
         this->_local = {}; // lifetime
      }
      if (prior_data) {
         try {
            auto src   = prior_data;
            auto dst   = this->_local.data();
            auto count = first_n;
            if (count) {
               this->_transfer_storage(src, dst, count);
            }
            count = this->size() - first_n - skip_next;
            if (count) {
               src += first_n + skip_next;
               dst += first_n;
               this->_transfer_storage(src, dst, count);
            }
         } catch (...) {
            this->_allocated = {
               .capacity = prior_capacity,
               .data     = prior_data,
            };
            throw;
         }
         for (size_t i = 0; i < skip_next; ++i) {
            std::destroy_at(prior_data + first_n + i);
         }
         this->_allocator.deallocate(prior_data, prior_capacity);
      }
      this->_set_storage_is_local();
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_resize_without_initializing(size_type size_after) {
      size_type size_prior = this->size();
      if (size_prior == size_after)
         return;
      if (size_after < size_prior) {
         value_type* data = this->data();
         for (size_type i = size_after; i < size_prior; ++i) {
            std::destroy_at(data + i);
         }
         this->_set_size(size_after);
         return;
      }
      this->reserve(size_after);
      value_type* data = this->data();
      this->_set_size(size_after);
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_set_capacity_to(size_type capacity) {
      const bool prior_local = _is_storage_local();
      if (!prior_local && capacity <= local_capacity) {
         this->_move_storage_to_local();
         return;
      }
      value_type* prior_data  = nullptr;
      #if __cpp_lib_allocate_at_least >= 202302L
         auto [after_data, after_capacity] = this->_allocator.allocate_at_least(capacity);
      #else
         auto after_data     = this->_allocator.allocate(capacity);
         auto after_capacity = capacity;
      #endif
      if (prior_local) {
         prior_data = this->_local.data();
      } else {
         prior_data = this->_allocated.data;
      }
      try {
         this->_transfer_storage(prior_data, after_data, this->size());
      } catch (...) {
         this->_allocator.deallocate(after_data, after_capacity);
         throw;
      }
      if (!prior_local && prior_data) {
         this->_allocator.deallocate(prior_data, this->_allocated.capacity);
      }
      this->_allocated = {
         .capacity = after_capacity,
         .data     = after_data,
      };
      this->_set_storage_is_heap();
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_transfer_storage(value_type* src, value_type* dst, size_type count) {
      if (count == 0) {
         return;
      } else {
         assert(!!src);
      }
      assert(!!dst);
      //
      // We move in order from last to first. This allows us to use this both for moving 
      // storage wholesale (between local and heap), and for insertions into the middle 
      // of the vector (to displace elements at and after the insertion point forward by 
      // one slot).
      //
      if constexpr (std::is_trivially_move_assignable_v<value_type>) {
         if (!std::is_constant_evaluated()) {
            std::memmove(dst, src, count * sizeof(value_type));
            return;
         }
      }
      if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
         while (count--) {
            std::construct_at(dst + count, std::move(src[count]));
         }
      } else {
         size_type remaining = count;
         try {
            while (remaining--) {
               std::construct_at(dst + remaining, std::move(src[remaining]));
            }
         } catch (...) {
            //
            // Undo those moves that succeeded (and hope we don't hit another exception).
            //
            while (count-- > remaining) {
               src[count] = std::move(dst[count]);
            }
            throw;
         }
      }
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::capacity() const noexcept {
      if (_is_storage_local()) {
         return local_capacity;
      } else {
         return this->_allocated.capacity;
      }
   }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::value_type* CLASS_NAME::data() const noexcept {
      if (_is_storage_local()) {
         return this->_local.data();
      } else {
         return this->_allocated.data;
      }
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type* CLASS_NAME::data() noexcept {
      return const_cast<value_type*>(std::as_const(*this).data());
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::empty() const noexcept {
      return this->size() == 0;
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::size() const noexcept {
      return this->_size_and_flags & ~((size_type)1 << _heap_bit_index);
   }

   #pragma region Element access
      TEMPLATE_PARAMS
      constexpr const typename CLASS_NAME::value_type& CLASS_NAME::at(size_type i) const {
         if (i >= this->size())
            throw std::out_of_range("lu::small_vector::at: access at or past size");
         return this->data()[i];
      }

      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::value_type& CLASS_NAME::at(size_type i) {
         return const_cast<value_type&>(std::as_const(*this).at(i));
      }

      TEMPLATE_PARAMS
      constexpr const typename CLASS_NAME::value_type& CLASS_NAME::back() const {
         auto i = this->size();
         assert(i > 0);
         return this->data()[i - 1];
      }

      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::value_type& CLASS_NAME::back() {
         return const_cast<value_type&>(std::as_const(*this).back());
      }

      TEMPLATE_PARAMS
      constexpr const typename CLASS_NAME::value_type& CLASS_NAME::front() const {
         assert(!this->empty());
         return this->data()[0];
      }

      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::value_type& CLASS_NAME::front() {
         return const_cast<value_type&>(std::as_const(*this).front());
      }

      TEMPLATE_PARAMS
      constexpr const typename CLASS_NAME::value_type& CLASS_NAME::operator[](size_type i) const noexcept {
         return this->data()[i];
      }

      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::value_type& CLASS_NAME::operator[](size_type i) noexcept {
         return const_cast<value_type&>(std::as_const(*this).operator[](i));
      }
   #pragma endregion

   #pragma region Reallocating
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::reserve(size_type new_capacity) {
         this->_constant_evaluation_init_local();
         if (this->capacity() >= new_capacity)
            return;
         if (new_capacity > max_size())
            throw std::length_error("capacity exceeds largest theoretically supported size");
         this->_set_capacity_to(new_capacity);
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::resize(size_type size_after) {
         size_type   size_prior = this->size();
         this->_resize_without_initializing(size_after);
         value_type* data       = this->data();
         for (size_type i = size_prior; i < size_after; ++i) {
            std::construct_at(data + i);
         }
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::resize(size_type size_after, const value_type& src) {
         size_type   size_prior = this->size();
         this->_resize_without_initializing(size_after);
         value_type* data       = this->data();
         for (size_type i = size_prior; i < size_after; ++i) {
            std::construct_at(data + i, src);
         }
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::shrink_to_fit() {
         const size_type capacity = this->capacity();
         const size_type size     = this->size();
         assert(size <= capacity);
         if (capacity != size)
            this->_set_capacity_to(size);
      }
   #pragma endregion
      
   #pragma region Modification
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::clear() {
         const auto size = this->size();
         if (!size)
            return;
         value_type* data = this->data();
         for (size_type i = 0; i < size; ++i) {
            std::destroy_at(data + i);
         }
         this->_set_size(0);
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::erase(const_iterator it) {
         assert(it >= this->begin());
         assert(it <  this->cend());
         const size_t subject = std::distance(this->cbegin(), it);
         const size_t size    = this->size();
         if (size - 1 < LocalCapacity && !_is_storage_local()) {
            //
            // We're currently on the heap, but should move to local as a 
            // result of removing one element.
            //
            this->_move_storage_to_local(subject, 1);
         } else {
            auto* data = this->data();
            for (size_t i = subject; i + 1 < size; ++i) {
               data[i] = std::move(data[i + 1]);
            }
            std::destroy_at(data + (size - 1));
         }
         this->_set_size(size - 1);
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::erase(const_iterator start, const_iterator end) {
         if (start == end)
            return;
         assert(start >= this->begin());
         assert(end   >  start);
         assert(end   <  this->cend());
         const size_t start_i = std::distance(this->cbegin(), start);
         const size_t end_i   = std::distance(this->cbegin(), end);
         const size_t removed = end_i - start_i;
         const size_t size    = this->size();
         if (size - removed < LocalCapacity && !_is_storage_local()) {
            //
            // We're currently on the heap, but should move to local as a 
            // result of removing one element.
            //
            this->_move_storage_to_local(start_i, removed);
         } else {
            auto* data = this->data();
            for (size_t i = start_i; i < size - removed; ++i) {
               data[i] = std::move(data[i + removed]);
            }
            data += (size - removed - 1);
            for (size_t i = 0; i < removed; ++i, ++data) {
               std::destroy_at(data);
            }
         }
         this->_set_size(size - 1);
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::insert(const_iterator at, const value_type& v) {
         this->insert(std::distance(this->begin(), at), v);
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::insert(const_iterator at, value_type&& v) {
         this->insert(std::distance(this->cbegin(), at), std::move(v));
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::insert(size_t at, const value_type& v) {
         const auto new_size     = this->size();
         const auto new_capacity = allocation_policy::template get_next_capacity<max_capacity>(new_size + 1, this->capacity());
         this->reserve(new_capacity);
         value_type* data = this->data();
         if (new_size > 1) {
            this->_transfer_storage(data + at, data + at + 1, new_size - at);
         }
         std::construct_at(&data[at], v);
         ++this->_size_and_flags;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::insert(size_t at, value_type&& v) {
         const auto new_size     = this->size();
         const auto new_capacity = allocation_policy::template get_next_capacity<max_capacity>(new_size + 1, this->capacity());
         this->reserve(new_capacity);
         value_type* data = this->data();
         if (new_size > 1) {
            this->_transfer_storage(data + at, data + at + 1, new_size - at);
         }
         std::construct_at(&data[at], std::move(v));
         ++this->_size_and_flags;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::push_back(const value_type& v) {
         const auto i = this->size();
         const auto c = allocation_policy::template get_next_capacity<max_capacity>(i + 1, this->capacity());
         this->reserve(c);
         std::construct_at(this->data() + i, v);
         ++this->_size_and_flags;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::push_back(value_type&& v) {
         const auto i = this->size();
         const auto c = allocation_policy::template get_next_capacity<max_capacity>(i + 1, this->capacity());
         this->reserve(c);
         std::construct_at(this->data() + i, std::move(v));
         ++this->_size_and_flags;
      }

      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::pop_back() {
         const auto size = this->size();
         assert(size != 0);
         std::destroy_at(this->data() + (this->size() - 1));
         if (size > 0)
            --this->_size_and_flags;
      }


      TEMPLATE_PARAMS
      template<typename... Args>
      constexpr CLASS_NAME::value_type& CLASS_NAME::emplace(const_iterator at, Args&&... args) {
         return this->emplace(std::distance(this->begin(), at), std::forward<Args>(args)...);
      }

      TEMPLATE_PARAMS
      template<typename... Args>
      constexpr CLASS_NAME::value_type& CLASS_NAME::emplace(size_type at, Args&&... args) {
         const auto new_size     = this->size();
         const auto new_capacity = allocation_policy::template get_next_capacity<max_capacity>(new_size + 1, this->capacity());
         this->reserve(new_capacity);
         value_type* data = this->data();
         if (new_size > 1) {
            this->_transfer_storage(data + at, data + at + 1, new_size - at);
         }
         std::construct_at(data + at, std::forward<Args>(args)...);
         ++this->_size_and_flags;
      }

      TEMPLATE_PARAMS
      template<typename... Args>
      constexpr CLASS_NAME::value_type& CLASS_NAME::emplace_back(Args&&... args) {
         const auto i = this->size();
         const auto c = allocation_policy::template get_next_capacity<max_capacity>(i + 1, this->capacity());
         this->reserve(c);
         std::construct_at(this->data() + i, std::forward<Args>(args)...);
         ++this->_size_and_flags;
      }
   #pragma endregion

   #pragma region Operators
      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::operator==(const small_vector& other) const noexcept {
         const size_type size = this->size();
         if (other.size() != size)
            return false;
         const value_type* data_a = this->data();
         const value_type* data_b = other.data();
         for (size_type i = 0; i < size; ++i)
            if (data_a[i] != data_b[i])
               return false;
         return true;
      }

      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::operator!=(const small_vector& other) const noexcept {
         const size_type size = this->size();
         if (other.size() != size)
            return true;
         const value_type* data_a = this->data();
         const value_type* data_b = other.data();
         for (size_type i = 0; i < size; ++i)
            if (data_a[i] != data_b[i])
               return true;
         return false;
      }
   #pragma endregion
      
   #pragma region Iterators
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_iterator CLASS_NAME::cbegin() const noexcept {
         return const_iterator(this->data());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::iterator CLASS_NAME::begin() noexcept {
         return iterator(this->data());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_iterator CLASS_NAME::cend() const noexcept {
         return const_iterator(this->data() + this->size());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::iterator CLASS_NAME::end() noexcept {
         return iterator(this->data() + this->size());
      }


      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_reverse_iterator CLASS_NAME::crbegin() const {
         return std::make_reverse_iterator(end());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::reverse_iterator CLASS_NAME::rbegin() {
         return std::make_reverse_iterator(end());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_reverse_iterator CLASS_NAME::crend() const {
         return std::make_reverse_iterator(begin());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::reverse_iterator CLASS_NAME::rend() {
         return std::make_reverse_iterator(begin());
      }
   #pragma endregion
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")


