#pragma once
#include "./low_footprint_string.h"
#include <limits>
#include <new> // std::bad_alloc
#include <stdexcept>
#if _DEBUG
   #include <cassert>
#endif

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename CharT, typename Traits, typename Allocator>
#define CLASS_NAME basic_low_footprint_string<CharT, Traits, Allocator>

namespace lu {
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_destroy_data() {
      if constexpr (!std::is_trivially_destructible_v<value_type>) {
         for (size_type i = 0; i < this->_size; ++i)
            this->_data[i].~value_type();
      }
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_deallocate() {
      std::allocator_traits<allocator_type>::deallocate(this->_allocator, this->_data, this->_size + 1); // plus 1 for the null terminator
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_assign(const value_type* src_data, size_type src_size) {
      this->_discard_and_resize(src_size);
      if (this->_data)
         traits_type::copy(this->_data, src_data, src_size);
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_discard_and_resize(size_type size) {
      if (this->_size == size)
         return;
      value_type* buffer = nullptr;
      if (size) {
         #if _DEBUG
            assert(size < this->max_size());
         #endif
         buffer = this->_allocator.allocate(size + 1);
         if (!buffer)
            throw std::bad_alloc();
         buffer[size] = value_type{0};
      }
      if (this->_data) {
         this->_destroy_data();
         this->_deallocate();
      }
      this->_data = buffer;
      this->_size = size;
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_resize(size_type size) {
      if (this->_size == size)
         return;
      value_type* buffer = nullptr;
      if (size) {
         #if _DEBUG
            assert(size < this->max_size());
         #endif
         buffer = this->_allocator.allocate(size + 1);
         if (!buffer)
            throw std::bad_alloc();
         buffer[size] = value_type{ 0 };
      }
      if (this->_data) {
         traits_type::copy(buffer, this->_data, std::min(size, this->_size));
         this->_destroy_data();
         this->_deallocate();
      }
      this->_data = buffer;
      this->_size = size;
   }

   #pragma region Constructors (non-copy/move)
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::basic_low_footprint_string(size_type count, value_type ch, const allocator_type& allocator)
         : _allocator(allocator)
      {
         this->_resize(count);
         traits_type::assign(this->_data, count, ch);
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::basic_low_footprint_string(const value_type* data, size_type size, const allocator_type& allocator)
         : _allocator(allocator)
      {
         #if _DEBUG
            assert(!!data == !!size);
         #endif
         _assign(data, size);
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::basic_low_footprint_string(const value_type* data, const allocator_type& allocator)
         : _allocator(allocator)
      {
         if (data)
            this->_assign(data, traits_type::length(data));
      }
   #pragma endregion
   #pragma region Constructors (copy/move)
      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr CLASS_NAME::basic_low_footprint_string(const StringLike& other)
         requires concepts::supports_string_like_interface<StringLike, value_type>
      :
         basic_low_footprint_string(other, allocator_type{})
      {}

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::basic_low_footprint_string(basic_low_footprint_string&& other) noexcept : basic_low_footprint_string(std::move(other), allocator_type{}) {}

      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr CLASS_NAME::basic_low_footprint_string(const StringLike& other, const allocator_type& allocator)
         requires concepts::supports_string_like_interface<StringLike, value_type>
      :
         _allocator(allocator)
      {
         #if _DEBUG
            assert(!!other.data() == !!other.size());
         #endif
         this->_assign(other.data(), other.size());
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::basic_low_footprint_string(basic_low_footprint_string&& other, const allocator_type& allocator)
         : _allocator(allocator)
      {
         this->_data = other._data;
         this->_size = other._size;
         other._data = nullptr;
         other._size = 0;
      }
   #pragma endregion

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::~basic_low_footprint_string() {
      this->clear();
   }
      
   #pragma region Element access
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_pointer CLASS_NAME::c_str() const noexcept {
         return this->_data;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::const_pointer CLASS_NAME::data() const noexcept {
         return this->_data;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::pointer CLASS_NAME::data() noexcept {
         return this->_data;
      }

      TEMPLATE_PARAMS
      const CLASS_NAME::value_type& CLASS_NAME::at(size_type i) const {
         if (i < 0 || i >= this->_size)
            throw std::out_of_range("low-footprint string access out of range");
         return this->_data[i];
      }
      TEMPLATE_PARAMS
      CLASS_NAME::value_type& CLASS_NAME::at(size_type i) {
         if (i < 0 || i >= this->_size)
            throw std::out_of_range("low-footprint string access out of range");
         return this->_data[i];
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type CLASS_NAME::back() const {
         #if _DEBUG
            assert(!this->empty());
         #endif
         return this->_data[this->_size - 1];
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::back() {
         #if _DEBUG
            assert(!this->empty());
         #endif
         return this->_data[this->_size - 1];
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type CLASS_NAME::front() const {
         #if _DEBUG
            assert(!this->empty());
         #endif
         return this->_data[0];
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::front() {
         #if _DEBUG
            assert(!this->empty());
         #endif
         return this->_data[0];
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type CLASS_NAME::operator[](size_type i) const {
         #if _DEBUG
            assert(i >= 0 && i < this->_size);
         #endif
         return this->_data[i];
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::operator[](size_type i) {
         #if _DEBUG
            assert(i >= 0 && i < this->_size);
         #endif
         return this->_data[i];
      }
   #pragma endregion
   #pragma region Capacity
      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::empty() const noexcept {
         return this->_data == nullptr;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::size_type CLASS_NAME::size() const noexcept {
         return this->_size;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::size_type CLASS_NAME::length() const noexcept {
         return this->_size;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::size_type CLASS_NAME::max_size() const noexcept {
         return std::numeric_limits<size_t>::max() / sizeof(value_type);
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::size_type CLASS_NAME::capacity() const noexcept {
         return this->_size;
      }
   #pragma endregion
   #pragma region Modifiers
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::clear() {
         if (this->_data) {
            this->_destroy_data();
            this->_deallocate();
         }
         this->_data = nullptr;
         this->_size = 0;
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::push_back(value_type ch) {
         size_type i = this->size();
         this->_resize(i + 1);
         this->_data[i] = ch;
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::pop_back() {
         #if _DEBUG
            assert(!this->empty());
         #endif
         this->_resize(this->size() - 1);
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::resize(size_type size) {
         this->_resize(size);
      }

      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr CLASS_NAME& CLASS_NAME::assign(const StringLike& other)
         requires concepts::supports_string_like_interface<StringLike, value_type>
      {
         this->_assign(other.data(), other.size());
         return *this;
      }
      //
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::assign(size_type size, value_type ch) {
         this->_discard_and_resize(size);
         traits_type::assign(this->_data, size, ch);
         return *this;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::assign(const value_type* data, size_type size) {
         this->_assign(data, size);
         return *this;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::assign(const value_type* data) {
         return this->assign(data, traits_type::length(data));
      }

      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr CLASS_NAME& CLASS_NAME::operator+=(const StringLike& other)
         requires concepts::supports_string_like_interface<StringLike, value_type>
      {
         if (other.empty())
            return;
         const size_type this_size = this->size();
         const size_type that_size = other.size();
         this->_resize(this_size + that_size);
         traits_type::copy(this->_data + this_size, other.data(), that_size);
         return *this;
      }
      //
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::operator+=(value_type ch) {
         this->push_back(ch);
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::operator+=(value_type* src) {
         if (!src)
            return;
         const size_type this_size = this->size();
         const size_type that_size = traits_type::length(src);
         this->_resize(this_size + that_size);
         traits_type::copy(this->_data + this_size, src, that_size);
         return *this;
      }
   #pragma endregion
   #pragma region Compare
      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr int CLASS_NAME::compare(const StringLike& other) const noexcept
         requires concepts::supports_string_like_interface<StringLike, value_type>
      {
         const size_type other_size = other.size();
         const size_type min_size   = std::min(this->_size, other_size);
         const int       compared   = traits_type::compare(this->_data, other.data(), (std::size_t)min_size);
         if (compared)
            return compared;
         return this->_size < other_size ? -1 : 1;
      }

      TEMPLATE_PARAMS
      constexpr int CLASS_NAME::compare(const value_type* other) const noexcept {
         const size_type other_size = traits_type::length(other);
         const size_type min_size   = std::min(this->_size, other_size);
         const int       compared   = traits_type::compare(this->_data, other, (std::size_t)min_size);
         if (compared)
            return compared;
         return this->_size < other_size ? -1 : 1;
      }

      TEMPLATE_PARAMS
      template<typename StringLike>
      constexpr bool CLASS_NAME::operator==(const StringLike& other) const noexcept
         requires concepts::supports_string_like_interface<StringLike, value_type>
      {
         if (this->_size != other.size())
            return false;
         if (this->_size == 0)
            return true;
         return traits_type::compare(this->_data, other.data(), (std::size_t)this->_size) == 0;
      }

      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::operator==(const value_type* other) const noexcept {
         if (!!this->_data != !!other)
            return false;
         if (!other)
            return true;
         for (size_type i = 0; i < this->_size; ++i) {
            value_type a = this->_data[i];
            value_type b = other[i];
            if (!traits_type::eq(a, b))
               return false;
            if (!b)
               return false;
         }
         return true;
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