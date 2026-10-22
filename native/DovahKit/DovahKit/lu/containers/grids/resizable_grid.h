#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

namespace lu {
   template<typename T, typename CoordinateType = int32_t> requires std::is_signed_v<CoordinateType>
   class resizable_grid {
      public:
         using value_type      = T;
         using coordinate_type = CoordinateType;
         using length_type     = std::make_unsigned_t<CoordinateType>;
         using size_type       = std::size_t;

      protected:
         template<bool Const> class _base_iterator {
            friend resizable_grid;
            using  target_type = std::conditional_t<Const, std::add_const_t<value_type>, std::remove_const_t<value_type>>;
            protected:
               target_type* target = nullptr;

               _base_iterator() {}
               _base_iterator(target_type* v) : target(v) {}

            public:
               target_type& operator*() { return *target; }
               target_type& operator*() const { return *target; }

               bool operator==(const _base_iterator&) const noexcept = default;

               _base_iterator& operator++() { ++target; return *this; }
               _base_iterator& operator--() { --target; return *this; }
               _base_iterator& operator+=(size_t n) { target += n; return *this; }
               _base_iterator& operator-=(size_t n) { target -= n; return *this; }

               _base_iterator operator++(int) const { return _base_iterator(*this)++; }
               _base_iterator operator--(int) const { return _base_iterator(*this)--; }
               _base_iterator operator+(size_t n) const { return _base_iterator(*this) += n; }
               _base_iterator operator-(size_t n) const { return _base_iterator(*this) -= n; }
         };
      public:
         using iterator = _base_iterator<false>;
         using const_iterator = _base_iterator<true>;

      protected:
         value_type* _data = nullptr;
         length_type _w    = 0;
         length_type _h    = 0;

         __declspec(allocator) static constexpr value_type* _alloc(size_t count) {
            return new value_type[count];
         }
         static constexpr void _free(value_type* memory) {
            delete[] memory;
         }
         
         constexpr void _resize_impl(length_type new_width, length_type new_height, auto NewStripFunctor, auto PreservedStripFunctor);

      public:
         constexpr resizable_grid() {}
         template<typename U, typename V> requires (
            (std::is_convertible_v<U, length_type> || std::is_same_v<U, length_type>)
         && (std::is_convertible_v<V, length_type> || std::is_same_v<V, length_type>)
         )
         constexpr resizable_grid(U w, V h) {
            this->resize((length_type)w, (length_type)h);
         }
         constexpr ~resizable_grid() {
            this->clear_and_collapse();
         }

         constexpr size_type area() const noexcept { return (size_type)width() * (size_type)height(); }
         constexpr const value_type* data() const noexcept { return (value_type*)this->_data; }
         constexpr value_type* data() noexcept { return (value_type*)this->_data; }
         constexpr bool empty() const noexcept { return this->area() != 0; }

         constexpr length_type width() const noexcept { return _w; }
         constexpr length_type height() const noexcept { return _h; }

         constexpr coordinate_type left() const noexcept { return -(this->width() / 2); }
         constexpr coordinate_type right() const noexcept { return (this->width() / 2) - (this->width() % 2 ? 0 : 1); }
         constexpr coordinate_type top() const noexcept { return -(this->height() / 2); }
         constexpr coordinate_type bottom() const noexcept { return (this->height() / 2) - (this->height() % 2 ? 0 : 1); }

         #pragma region Iterator-based access
            iterator begin() { return iterator{ _data }; }
            iterator end() { return iterator{ _data + area() }; }
            const_iterator cbegin() const { return const_iterator{ _data }; }
            const_iterator cend() const { return const_iterator{ _data + area() }; }
            const_iterator begin() const { return cbegin(); }
            const_iterator end() const { return cend(); }
         #pragma endregion

      public:
         constexpr bool contains_coordinate(coordinate_type x, coordinate_type y) {
            if (x < left() || x > right())
               return false;
            if (y < top() || y > bottom())
               return false;
            return true;
         }

         constexpr const value_type& item(coordinate_type x, coordinate_type y) const noexcept {
            return this->data()[(x - left()) + ((y - top()) * this->width())];
         }
         constexpr value_type& item(coordinate_type x, coordinate_type y) noexcept {
            return this->data()[(x - left()) + ((y - top()) * this->width())];
         }

         constexpr void clear();
         constexpr void clear_and_collapse();

         constexpr void resize(length_type new_width, length_type new_height) requires (std::is_default_constructible_v<value_type> && std::is_move_constructible_v<value_type>);
         constexpr void resize(length_type new_width, length_type new_height, const value_type& v) requires (std::is_copy_constructible_v<value_type>&& std::is_move_constructible_v<value_type>);
   };
}

#include "./resizable_grid.inl"