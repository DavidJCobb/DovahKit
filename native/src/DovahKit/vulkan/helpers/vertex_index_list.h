#pragma once
#include <array>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <stdlib.h>
#include <type_traits>

namespace vulkanDK {
   namespace impl::vertex_index_list {
      template<typename T> concept is_list = requires(T x) {
         typename T::value_type;
         typename T::size_type;
         requires std::is_same_v<typename T::size_type, size_t>;
         { x.size() } -> std::same_as<size_t>;
         { x.data() } -> std::same_as<typename T::value_type*>;
         { x[0] } -> std::same_as<typename T::value_type&>;
      };
      template<typename T> concept value_type_is_usable = requires {
         typename T::value_type;
         requires std::is_same_v<typename T::value_type, uint16_t> || std::is_same_v<typename T::value_type, uint32_t>;
      };
   }

   class vertex_index_list {
      public:
         enum class value_type {
            thin,
            wide,
         };

         using thin_type = uint16_t;
         using wide_type = uint32_t;

         inline static size_t value_size(value_type t) noexcept {
            if (t == value_type::thin)
               return sizeof(thin_type);
            return sizeof(uint32_t);
         }

         template<typename vt> class typed_range;

         template<typename vt> class _iterator {
            friend class typed_range<vt>;
            public:
               using iterator_category = std::forward_iterator_tag;
               using difference_type   = std::ptrdiff_t;
               using value_type = vt;
               using pointer    = value_type*;
               using reference  = value_type&;
            private:
               pointer target = nullptr;
            protected:
               _iterator() {}
               _iterator(reference r) : target(&r) {}

            public:
               reference operator*() const { return *target; }
               pointer operator->() { return target; }

               _iterator& operator++() {
                  ++target;
                  return *this;
               }
               _iterator operator++(int) {
                  auto old = *this;
                  ++(*this);
                  return old;
               }

               inline auto operator<=>(const _iterator& other) const noexcept {
                  return target <=> other.target;
               }
         };

         template<typename vt> class typed_range {
            friend class vertex_index_list;
            //
            static constexpr bool is_const = std::is_const_v<vt>;
            using target_type      = std::conditional_t<is_const, const vertex_index_list&, vertex_index_list&>;
            using plain_value_type = std::decay_t<vt>;
            using const_value_type = std::add_const_t<plain_value_type>;
            //
            private:
               target_type target;
            protected:
               typed_range(target_type t) : target(t) {}

            public:
               using iterator       = _iterator<plain_value_type>;
               using const_iterator = _iterator<const_value_type>;

            public:
               iterator begin() requires(!is_const) { return iterator((plain_value_type*)target._data.untyped); }
               iterator end() requires(!is_const) {
                  std::intptr_t address = (std::intptr_t)target._data.untyped + target.size_in_bytes();
                  return iterator((plain_value_type*)address);
               }
               const_iterator begin() const { return const_iterator((const_value_type*)target._data.untyped); }
               const_iterator end() const {
                  std::intptr_t address = (std::intptr_t)target._data.untyped + target.size_in_bytes();
                  return const_iterator((const_value_type*)address);
               }

               const_iterator cbegin() const { return const_iterator((const_value_type*)target._data.untyped); }
               const_iterator cend() const {
                  std::intptr_t address = (std::intptr_t)target._data.untyped + target.size_in_bytes();
                  return const_iterator((const_value_type*)address);
               }

               plain_value_type& operator[](size_t i) requires (!is_const) {
                  auto address = (std::intptr_t)target._data.untyped + (sizeof(plain_value_type) * i);
                  return *(plain_value_type*)address;
               }
               const plain_value_type& operator[](size_t i) const {
                  auto address = (std::intptr_t)target._data.untyped + (sizeof(plain_value_type) * i);
                  return *(const plain_value_type*)address;
               }
         };

      protected:
         union {
            void* untyped = nullptr;
            //
            thin_type* thin;
            wide_type* wide;
         } _data;
         size_t _capacity = 0;
         size_t _size     = 0;
         value_type _type = value_type::thin;

      public:
         vertex_index_list() {}
         ~vertex_index_list();

         explicit vertex_index_list(size_t count, thin_type); // multiple of the same element
         explicit vertex_index_list(size_t count, wide_type); // multiple of the same element

         vertex_index_list(const vertex_index_list&);
         vertex_index_list& operator=(const vertex_index_list&);

         vertex_index_list(vertex_index_list&&) noexcept;
         vertex_index_list& operator=(vertex_index_list&&) noexcept;

         #pragma region templated constructors
         template<typename T> requires (impl::vertex_index_list::is_list<T> && impl::vertex_index_list::value_type_is_usable<T>)
         vertex_index_list(const T& list) {
            using V = T::value_type;
            constexpr auto vt = std::is_same_v<V, thin_type> ? value_type::thin : value_type::wide;
            //
            this->_type = vt;
            this->resize(list.size());
            memcpy(this->_data.thin, list.data(), list.size() * sizeof(V));
         }

         // allow assigning some_list<uint16_t> or some_list<uint32_t>
         template<typename T> requires (impl::vertex_index_list::is_list<T> && impl::vertex_index_list::value_type_is_usable<T>)
         vertex_index_list& operator=(const T& list) {
            using V = T::value_type;
            constexpr auto vt = std::is_same_v<V, thin_type> ? value_type::thin : value_type::wide;
            //
            if (this->_type != vt || this->_size < list.size()) {
               if (this->_data.untyped)
                  delete this->_data.untyped;
               this->_data.untyped = nullptr;
               //
               this->_type = vt;
               this->_size = list.size();
               this->_capacity = list.size();
               if (list.size()) {
                  this->_data.untyped = malloc(list.size() * sizeof(V));
               } else {
                  return *this;
               }
            } else {
               this->_size = list.size();
            }
            memcpy(this->_data.untyped, list.data(), list.size() * sizeof(V));
            return *this;
         }

         vertex_index_list& operator=(const std::initializer_list<int>& list) {
            bool is_wide = false;
            for (auto& v : list) {
               if (v > std::numeric_limits<thin_type>::max()) {
                  is_wide = true;
                  break;
               }
            }
            //
            auto vt = !is_wide ? value_type::thin : value_type::wide;
            auto vs = !is_wide ? sizeof(thin_type) : sizeof(wide_type);
            //
            if (this->_type != vt || this->_size < list.size()) {
               delete this->_data.untyped;
               this->_data.untyped = nullptr;
               //
               this->_type     = vt;
               this->_size     = list.size();
               this->_capacity = list.size();
               if (list.size()) {
                  this->_data.untyped = malloc(list.size() * vs);
               } else {
                  return *this;
               }
            } else {
               this->_size = list.size();
            }
            size_t i = 0;
            for (auto& v : list) {
               if (is_wide) {
                  this->_data.wide[i++] = v;
               } else {
                  this->_data.thin[i++] = v;
               }
            }
            return *this;
         }
         #pragma endregion

         void clear();
         void set_type(value_type);

         void reserve(size_t);
         void resize(size_t);

         inline size_t capacity() const noexcept { return this->_capacity; }
         inline size_t size() const noexcept { return this->_size; }
         inline value_type type() const noexcept { return this->_type; }
         inline const void* data() const noexcept { return this->_data.untyped; }
         thin_type* thin_data() const; // returns nullptr if the list is wide; may also be nullptr if the list is empty
         wide_type* wide_data() const; // returns nullptr if the list is thin; may also be nullptr if the list is empty
         //
         inline size_t value_size() const noexcept { return value_size(this->_type); }
         inline size_t size_in_bytes() const noexcept { return this->value_size() * this->size(); }

         inline const wide_type operator[](size_t i) const noexcept {
            if (this->_type == value_type::thin)
               return this->_data.thin[i];
            return this->_data.wide[i];
         }

         inline size_t triangle_count() const noexcept { return this->size() / 3; }
         inline std::array<wide_type, 3> triangle_from(size_t i) const noexcept {
            if (this->_type == value_type::thin) {
               return std::array<wide_type, 3>{ this->_data.thin[i], this->_data.thin[i + 1], this->_data.thin[i + 2] };
            } else {
               return std::array<wide_type, 3>{ this->_data.wide[i], this->_data.wide[i + 1], this->_data.wide[i + 2] };
            }
         }
         inline std::array<wide_type, 3> triangle(size_t i) const noexcept {
            return this->triangle_from(i * 3);
         }

         typed_range<thin_type> as_thin_range() { return typed_range<thin_type>(*this); }
         const typed_range<const thin_type> as_thin_range() const noexcept { return typed_range<const thin_type>(*this); }
         typed_range<wide_type> as_wide_range() { return typed_range<wide_type>(*this); }
         const typed_range<const wide_type> as_wide_range() const noexcept { return typed_range<const wide_type>(*this); }
   };
}