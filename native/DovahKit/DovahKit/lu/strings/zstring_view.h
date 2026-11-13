#pragma once
#include <concepts>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <version>

namespace lu {
   // String view for null-terminated strings.
   template<typename CharT>
   class basic_zstring_view {
      public:
         using size_type     = size_t;
         using value_type    = CharT;
         using const_pointer = const value_type*;
         
         static constexpr size_type npos = std::numeric_limits<size_type>::max();

         using std_view_type = std::basic_string_view<CharT>;
      
      protected:
         const_pointer _data = nullptr;
         size_type     _size = 0;
      
      public:
         constexpr basic_zstring_view() {}
         
         #if __cpp_lib_constexpr_string >= 201907L
         constexpr
         #endif
         basic_zstring_view(const std::string& s) : _data(s.data()), _size(s.size()) {}
         
         constexpr basic_zstring_view(const_pointer s) : _data(s) {
            this->_size = 0;
            if (s)
               while (s[this->_size])
                  ++this->_size;
         }
         
         constexpr const_pointer c_str() const noexcept { return this->_data; }
         constexpr const_pointer data() const noexcept { return this->_data; }
         constexpr size_type size() const noexcept { return this->_size; }
         constexpr size_type length() const noexcept { return this->_size; }
         
         constexpr bool empty() const noexcept {
            if (this->_data)
               return this->_size == 0;
            return true;
         }
         
         constexpr operator std_view_type() const noexcept {
            return std_view_type(this->_data, this->_size);
         }
         constexpr bool operator==(const basic_zstring_view& other) const noexcept {
            return (std_view_type)*this == (std_view_type)other;
         }
         
         constexpr const value_type& operator[](size_t n) const noexcept {
            return this->_data[n];
         }
         constexpr const value_type& front() const noexcept { return this->_data[0]; }
         constexpr const value_type& back() const noexcept { return this->_data[this->_size - 1]; }
         
         constexpr void remove_prefix(size_t n) {
            if (std::is_constant_evaluated()) {
               if (n > this->_size)
                  throw;
            }
            this->_data += n;
            this->_size -= n;
         }
         
         constexpr bool starts_with(std_view_type) const;
         constexpr bool starts_with(value_type) const;
         
         constexpr bool ends_with(std_view_type) const;
         constexpr bool ends_with(value_type) const;
         
         #if __cpp_lib_string_contains >= 202011L
            constexpr bool contains(std_view_type) const;
            constexpr bool contains(value_type) const;
         #endif
         
         constexpr size_type find(std_view_type, size_type pos = 0) const noexcept;
         constexpr size_type find(value_type, size_type pos = 0) const noexcept;
         constexpr size_type find(const_pointer needle, size_type pos, size_type needle_size) const noexcept;
         
         constexpr size_type rfind(std_view_type, size_type pos = npos) const noexcept;
         constexpr size_type rfind(value_type, size_type pos = npos) const noexcept;
         constexpr size_type rfind(const_pointer needle, size_type pos, size_type needle_size) const noexcept;
         
         constexpr size_type find_first_of(std_view_type, size_type pos = 0) const noexcept;
         constexpr size_type find_first_of(value_type, size_type pos = 0) const noexcept;
         constexpr size_type find_first_of(const_pointer set, size_type pos, size_type set_size) const noexcept;
         
         constexpr size_type find_last_of(std_view_type, size_type pos = npos) const noexcept;
         constexpr size_type find_last_of(value_type, size_type pos = npos) const noexcept;
         constexpr size_type find_last_of(const_pointer set, size_type pos, size_type set_size) const noexcept;
         
         constexpr size_type find_first_not_of(std_view_type, size_type pos = 0) const noexcept;
         constexpr size_type find_first_not_of(value_type, size_type pos = 0) const noexcept;
         constexpr size_type find_first_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept;
         
         constexpr size_type find_last_not_of(std_view_type, size_type pos = npos) const noexcept;
         constexpr size_type find_last_not_of(value_type, size_type pos = npos) const noexcept;
         constexpr size_type find_last_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept;
   };

   using zstring_view = basic_zstring_view<char>;
} 

// facilitate `std::cout << my_zstring_view` and the like
namespace std {
   template<class CharT, class Traits>
   class basic_ostream; // forward-declaration
}

template<typename CharT>
inline std::basic_ostream<CharT, std::char_traits<CharT>>& operator<<(
   std::basic_ostream<CharT, std::char_traits<CharT>>& stream,
   lu::basic_zstring_view<CharT> str
) {
   return stream << (typename lu::basic_zstring_view<CharT>::std_view_type)str;
}

#include "./zstring_view.inl"