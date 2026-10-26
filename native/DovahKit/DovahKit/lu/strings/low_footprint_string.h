#pragma once
#include <iterator> // std::reverse_iterator, std::make_reverse_iterator
#include <memory> // std::allocator and friends
#include <string> // std::char_traits and friends
#include <string_view>
#include "../concepts/supports_string_like_interface.h"
#include "../iterators/pointer_based_iterator.h"
#include "../macros/NO_UNIQUE_ADDRESS.define.h"

namespace lu {
   template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
   class basic_low_footprint_string {
      public:
         using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<CharT>;
         using different_type = std::allocator_traits<allocator_type>::difference_type;
         using size_type      = std::allocator_traits<allocator_type>::size_type;
         using traits_type    = Traits;
         using value_type     = CharT;

         using reference       = value_type&;
         using const_reference = const value_type&;
         using pointer         = std::allocator_traits<allocator_type>::pointer;
         using const_pointer   = std::allocator_traits<allocator_type>::const_pointer;

         using iterator               = iterators::pointer_based_iterator<basic_low_footprint_string, value_type, false>;
         using const_iterator         = iterators::pointer_based_iterator<basic_low_footprint_string, value_type, true>;
         using reverse_iterator       = std::reverse_iterator<iterator>;
         using const_reverse_iterator = std::reverse_iterator<const_iterator>;

         using string_view_type = std::basic_string_view<value_type, traits_type>;

         static constexpr const size_type npos = (size_type)-1;

      protected:
         pointer   _data = nullptr;
         size_type _size = 0;
         LU_NO_UNIQUE_ADDRESS allocator_type _allocator;

      protected:
         constexpr void _destroy_data();
         constexpr void _deallocate();
         constexpr void _assign(const value_type*, size_type);
         constexpr void _discard_and_resize(size_type);
         constexpr void _resize(size_type);

      public:
         #pragma region Constructors (non-copy/move)
            constexpr basic_low_footprint_string() {}
            explicit constexpr basic_low_footprint_string(const allocator_type&);
            constexpr basic_low_footprint_string(size_type count, value_type ch, const allocator_type& = {});
            constexpr basic_low_footprint_string(const value_type*, size_type, const allocator_type& = {});
            constexpr basic_low_footprint_string(const value_type*, const allocator_type& = {});
            basic_low_footprint_string(std::nullptr_t) = delete;
         #pragma endregion
         #pragma region Constructors (copy/move)
            template<typename StringLike>
            constexpr basic_low_footprint_string(const StringLike&)
               requires concepts::supports_string_like_interface<StringLike, value_type>;

            constexpr basic_low_footprint_string(basic_low_footprint_string&&) noexcept;

            template<typename StringLike>
            constexpr basic_low_footprint_string(const StringLike&, const allocator_type&)
               requires concepts::supports_string_like_interface<StringLike, value_type>;

            constexpr basic_low_footprint_string(basic_low_footprint_string&&, const allocator_type&);

            template<typename StringLike>
            constexpr basic_low_footprint_string& operator=(const StringLike& other)
               requires concepts::supports_string_like_interface<StringLike, value_type>
            {
               return this->assign(other);
            }

            constexpr basic_low_footprint_string& operator=(const value_type* v) {
               return this->assign(v);
            }
         #pragma endregion

         constexpr ~basic_low_footprint_string();

         #pragma region Element access
            constexpr const_pointer c_str() const noexcept;
            constexpr const_pointer data() const noexcept;
            constexpr pointer data() noexcept;

            const value_type& at(size_type) const;
            value_type& at(size_type);
            constexpr value_type  back() const;
            constexpr value_type& back();
            constexpr value_type  front() const;
            constexpr value_type& front();

            constexpr value_type  operator[](size_type) const;
            constexpr value_type& operator[](size_type);
         #pragma endregion
         #pragma region Capacity
            constexpr bool empty() const noexcept;
            constexpr size_type size() const noexcept;
            constexpr size_type length() const noexcept;
            constexpr size_type max_size() const noexcept;
            constexpr size_type capacity() const noexcept;
         #pragma endregion
         #pragma region Modifiers
            constexpr void clear();
            constexpr void push_back(value_type);
            constexpr void pop_back();
            constexpr void resize(size_type);

            template<typename StringLike>
            constexpr basic_low_footprint_string& assign(const StringLike&)
               requires concepts::supports_string_like_interface<StringLike, value_type>;
            //
            constexpr basic_low_footprint_string& assign(size_type, value_type ch);
            constexpr basic_low_footprint_string& assign(const value_type*, size_type);
            constexpr basic_low_footprint_string& assign(const value_type*);

            template<typename StringLike>
            constexpr basic_low_footprint_string& operator+=(const StringLike&)
               requires concepts::supports_string_like_interface<StringLike, value_type>;
            //
            constexpr basic_low_footprint_string& operator+=(value_type);
            constexpr basic_low_footprint_string& operator+=(value_type*);
         #pragma endregion
         #pragma region Searching
            #pragma push_macro("LU_MAKE_FIND_FUNC")
            #undef  LU_MAKE_FIND_FUNC
            #define LU_MAKE_FIND_FUNC(_name_, _default_pos_) \
               template<typename StringLike> \
               constexpr size_type _name_(const StringLike& other, size_type pos = _default_pos_) const noexcept \
                  requires concepts::supports_string_like_interface<StringLike, value_type> \
               { \
                  return string_view_type(this->data(), this->size())._name_(other.data(), pos, other.size()); \
               } \
               \
               constexpr size_type _name_(const value_type* other, size_type pos, size_type count) const noexcept { \
                  return string_view_type(this->data(), this->size())._name_(other, pos, count); \
               } \
               constexpr size_type _name_(const value_type* other, size_type pos = _default_pos_) const noexcept { \
                  return string_view_type(this->data(), this->size())._name_(other, pos); \
               } \
               constexpr size_type _name_(value_type other, size_type pos = _default_pos_) const noexcept { \
                  return string_view_type(this->data(), this->size())._name_(other, pos); \
               }

            LU_MAKE_FIND_FUNC(find, 0)
            LU_MAKE_FIND_FUNC(rfind, npos)
            LU_MAKE_FIND_FUNC(find_first_of, 0)
            LU_MAKE_FIND_FUNC(find_first_not_of, 0)
            LU_MAKE_FIND_FUNC(find_last_of, npos)
            LU_MAKE_FIND_FUNC(find_last_not_of, npos)

            #undef  LU_MAKE_FIND_FUNC
            #pragma pop_macro("LU_MAKE_FIND_FUNC")
         #pragma endregion
         #pragma region Compare
            template<typename StringLike>
            constexpr int compare(const StringLike&) const noexcept
               requires concepts::supports_string_like_interface<StringLike, value_type>;

            constexpr int compare(const value_type*) const noexcept;

            template<typename StringLike>
            constexpr bool operator==(const StringLike&) const noexcept
               requires concepts::supports_string_like_interface<StringLike, value_type>;

            constexpr bool operator==(const value_type*) const noexcept;

            template<typename... Args>
            constexpr bool starts_with(Args&&... args) const noexcept {
               return string_view_type(this->data(), this->size()).starts_with(std::forward<Args>(args)...);
            }

            template<typename... Args>
            constexpr bool ends_with(Args&&... args) const noexcept {
               return string_view_type(this->data(), this->size()).ends_with(std::forward<Args>(args)...);
            }
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

   using low_footprint_string = basic_low_footprint_string<char>;
}

#include "../macros/NO_UNIQUE_ADDRESS.undef.h"
#include "./low_footprint_string.inl"