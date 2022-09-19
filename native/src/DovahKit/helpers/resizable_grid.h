#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

namespace cobb {
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
         
         constexpr void _resize_impl(length_type new_width, length_type new_height, auto NewStripFunctor, auto PreservedStripFunctor) {
            if (new_width == this->_w && new_height == this->_h)
               return;
            auto new_area = new_width * new_height;
            if (new_area == 0) {
               this->clear_and_collapse();
               return;
            }
            auto* move_to = _alloc(new_width * new_height);

            length_type copy_src_y_min =  0; // number of rows cropped off of the top
            length_type copy_src_y_max = _h;
            length_type copy_dst_y_min =  0; // number of new rows added to the top
            if (new_height > _h) {
               copy_dst_y_min = (new_height - _h) / 2;
            } else if (new_height < _h) {
               copy_src_y_min = (_h - new_height) / 2;
               copy_src_y_max = copy_src_y_min + new_height;
            }

            length_type copy_src_x_min =  0;
            length_type copy_src_x_cnt = _w;
            length_type copy_dst_x_min =  0;
            if (new_width > _w) {
               copy_dst_x_min = (new_width - _w) / 2;
            } else if (new_width < _w) {
               copy_src_x_min = (_w - new_width) / 2;
               copy_src_x_cnt = new_width;
            }

            length_type h1 = copy_src_y_min;
            length_type h2 = copy_dst_y_min;
            if (copy_dst_y_min) {
               NewStripFunctor(move_to, copy_dst_y_min * new_width);
            }
            for (; h1 < copy_src_y_max; ++h1, ++h2) {
               auto* src_row = &this->_data[h1 * this->_w];
               auto* dst_row = &move_to[h2 * new_width];

               length_type w1 = copy_src_x_min;
               length_type w2 = copy_dst_x_min;
               if (copy_dst_x_min) {
                  NewStripFunctor(dst_row, copy_dst_x_min);
               }
               PreservedStripFunctor(src_row + w1, dst_row + w2, copy_src_x_cnt);
               w2 += copy_src_x_cnt;
               if (w2 < new_width) {
                  NewStripFunctor(dst_row + w2, new_width - w2);
               }
            }
            if (h2 < new_height) {
               NewStripFunctor(move_to + (h2 * new_width), (new_height - h2) * new_width);
            }
            //
            // Done copying.
            //
            if (this->_data)
               _free(this->_data);
            this->_data = move_to;
            this->_w    = new_width;
            this->_h    = new_height;
         }

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

         constexpr length_type width() const noexcept { return _w; }
         constexpr length_type height() const noexcept { return _h; }
         constexpr size_type area() const noexcept { return (size_type)width() * (size_type)height(); }
         constexpr bool empty() const noexcept { return this->area() != 0; }

         constexpr coordinate_type left() const noexcept { return -(this->width() / 2); }
         constexpr coordinate_type right() const noexcept { return (this->width() / 2) - (this->width() % 2 ? 0 : 1); }
         constexpr coordinate_type top() const noexcept { return -(this->height() / 2); }
         constexpr coordinate_type bottom() const noexcept { return (this->height() / 2) - (this->height() % 2 ? 0 : 1); }

         constexpr value_type* data() noexcept { return (value_type*) this->_data; }
         constexpr const value_type* data() const noexcept { return (value_type*)this->_data; }

         constexpr value_type& at(coordinate_type x, coordinate_type y) noexcept {
            return this->data()[(x - left()) + ((y - top()) * this->width())];
         }
         constexpr const value_type& at(coordinate_type x, coordinate_type y) const noexcept {
            return this->data()[(x - left()) + ((y - top()) * this->width())];
         }

         iterator begin() { return iterator{ _data }; }
         iterator end() { return iterator{ _data + area() }; }
         const_iterator cbegin() const { return const_iterator{ _data }; }
         const_iterator cend() const { return const_iterator{ _data + area() }; }
         const_iterator begin() const { return cbegin(); }
         const_iterator end() const { return cend(); }

         constexpr void clear() {
            for (size_t i = 0; i < this->area(); ++i)
               this->_data[i] = value_type{};
         }
         constexpr void clear_and_collapse() {
            if (this->_data) {
               _free(this->_data);
               this->_data = nullptr;
            }
            this->_w = 0;
            this->_h = 0;
         }

         constexpr void resize(length_type new_width, length_type new_height) requires (std::is_default_constructible_v<value_type> && std::is_move_constructible_v<value_type>) {
            this->_resize_impl(
               new_width,
               new_height,
               [](value_type* base, size_t count) {
                  for(size_t i = 0; i < count; ++i)
                     std::construct_at<value_type>(base + i);
               },
               [](value_type* src, value_type* dst, length_type count) {
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(dst, src, sizeof(T) * (size_t)count);
                        return;
                     }
                  }
                  for (length_type i = 0; i < count; ++i) {
                     auto* target = &dst[i];
                     std::construct_at<value_type>(target, std::move(src[i]));
                  }
               }
            );
         }
         constexpr void resize(length_type new_width, length_type new_height, const value_type& v) requires (std::is_copy_constructible_v<value_type> && std::is_move_constructible_v<value_type>) {
            this->_resize_impl(
               new_width,
               new_height,
               [&v](value_type* base, size_t count) {
                  for(size_t i = 0; i < count; ++i)
                     std::construct_at<value_type>(base + i, v);
               },
               [](value_type* src, value_type* dst, length_type count) {
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(dst, src, sizeof(T) * (size_t)count);
                        return;
                     }
                  }
                  for (length_type i = 0; i < count; ++i)
                     std::construct_at<value_type>(&dst[i], std::move(src[i]));
               }
            );
         }
   };


   //
   // A resizable square grid centered on (0, 0).
   //
   template<typename T, typename CoordinateType = int32_t> requires std::is_signed_v<CoordinateType>
   class resizable_square_grid {
      public:
         using value_type      = T;
         using coordinate_type = CoordinateType;
         using length_type     = std::make_unsigned_t<CoordinateType>;
         using size_type       = std::size_t;
         
      protected:
         template<bool Const> class _base_iterator {
            friend resizable_square_grid;
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
         length_type _length = 0;

         __declspec(allocator) static constexpr value_type* _alloc(size_t count) {
            return new value_type[count];
         }
         static constexpr void _free(value_type* memory) {
            delete[] memory;
         }

         static constexpr value_type& _item_in_new_grid(value_type* buffer, length_type length, length_type x, length_type y) {
            return buffer[x + (y * length)];
         }
         constexpr value_type& _zero_based_item(length_type x, length_type y) noexcept {
            return this->data()[x + (y * this->length())];
         }

         constexpr void _resize_impl(length_type new_length, auto NewSlotFunctor, auto PreservedStripFunctor) {
            if (new_length == this->_length)
               return;
            if (new_length == 0) {
               this->clear_and_collapse();
               return;
            }
            auto* move_to = _alloc(new_length * new_length);
            if (new_length > this->_length) {
               //
               // Expanding the grid.
               //
               if (this->_length) {
                  length_type pad_a = (new_length - this->_length) / 2;
                  length_type pad_b = pad_a + this->_length;
                  //
                  for (length_type y2 = 0; y2 < pad_a; ++y2) // new rows on top
                     for (length_type x2 = 0; x2 < new_length; ++x2)
                        NewSlotFunctor(&_item_in_new_grid(move_to, new_length, x2, y2));
                  //
                  for (length_type y1 = 0, y2 = pad_a; y1 < this->_length; ++y1, ++y2) { // expanded rows in middle
                     for (length_type x2 = 0; x2 < pad_a; ++x2) // new columns on left
                        NewSlotFunctor(&_item_in_new_grid(move_to, new_length, x2, y2));
                     //
                     PreservedStripFunctor(&_item_in_new_grid(move_to, new_length, pad_a, y2), &this->_zero_based_item(0, y1), this->_length);
                     //
                     for (length_type x2 = pad_b; x2 < new_length; ++x2) // new columns on right
                        NewSlotFunctor(&_item_in_new_grid(move_to, new_length, x2, y2));
                  }
                  //
                  for (length_type y2 = pad_b; y2 < new_length; ++y2) // new rows on bottom
                     for (length_type x2 = 0; x2 < new_length; ++x2)
                        NewSlotFunctor(&_item_in_new_grid(move_to, new_length, x2, y2));
               } else {
                  //
                  // Grid was zero-size.
                  //
                  for (length_type y = 0; y < new_length; ++y)
                     for (length_type x = 0; x < new_length; ++x)
                        NewSlotFunctor(&_item_in_new_grid(move_to, new_length, x, y));
               }
            } else {
               //
               // Shrinking the grid.
               //
               length_type skip_a = (this->_length - new_length) / 2;
               length_type skip_b = skip_a + new_length;
               //
               for (length_type y1 = skip_a, y2 = 0; y1 < skip_b; ++y1, ++y2) {
                  PreservedStripFunctor(&_item_in_new_grid(move_to, new_length, 0, y2), &this->_zero_based_item(skip_a, y1), new_length);
               }
            }
            if (this->_data)
               _free(this->_data);
            this->_data   = move_to;
            this->_length = new_length;
         }

      public:
         constexpr resizable_square_grid() {}
         template<typename U> requires (std::is_convertible_v<U, length_type> || std::is_same_v<U, length_type>) constexpr resizable_square_grid(U l) {
            this->resize((length_type)l);
         }
         constexpr ~resizable_square_grid() {
            this->clear_and_collapse();
         }

         constexpr length_type length() const noexcept { return _length; }
         constexpr size_type area() const noexcept { return (size_type)length() * (size_type)length(); }
         constexpr bool empty() const noexcept { return this->length() != 0; }

         constexpr coordinate_type left() const noexcept { return -(this->length() / 2); }
         constexpr coordinate_type right() const noexcept { return (this->length() / 2) - (this->length() % 2 ? 0 : 1); }
         constexpr coordinate_type top() const noexcept { return left(); }
         constexpr coordinate_type bottom() const noexcept { return right(); }

         constexpr value_type* data() noexcept { return (value_type*) this->_data; }
         constexpr const value_type* data() const noexcept { return (value_type*)this->_data; }

         constexpr value_type& at(coordinate_type x, coordinate_type y) noexcept {
            auto half = this->length() / 2;
            return this->data()[(x + half) + ((y + half) * this->length())];
         }
         constexpr const value_type& at(coordinate_type x, coordinate_type y) const noexcept {
            auto half = this->length() / 2;
            return this->data()[(x + half) + ((y + half) * this->length())];
         }
         
         iterator begin() { return iterator{ _data }; }
         iterator end() { return iterator{ _data + area() }; }
         const_iterator cbegin() const { return const_iterator{ _data }; }
         const_iterator cend() const { return const_iterator{ _data + area() }; }
         const_iterator begin() const { return cbegin(); }
         const_iterator end() const { return cend(); }

         constexpr void clear() {
            for (size_t i = 0; i < this->area(); ++i)
               this->_data[i] = value_type{};
         }
         constexpr void clear_and_collapse() {
            if (this->_data) {
               _free(this->_data);
               this->_data = nullptr;
            }
            this->_length = 0;
         }

         constexpr void resize(length_type new_length) requires (std::is_default_constructible_v<value_type> && std::is_move_constructible_v<value_type>) {
            this->_resize_impl(
               new_length,
               [](value_type* item) {
                  std::construct_at<value_type>(item);
               },
               [](value_type* dst, value_type* src, length_type count) {
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(dst, src, sizeof(T) * (size_t)count);
                        return;
                     }
                  }
                  for (length_type i = 0; i < count; ++i)
                     std::construct_at<value_type>(&dst[i], std::move(src[i]));
               }
            );
         }
         constexpr void resize(length_type new_length, const value_type& v) requires (std::is_copy_constructible_v<value_type> && std::is_move_constructible_v<value_type>) {
            this->_resize_impl(
               new_length,
               [&v](value_type* item) {
                  std::construct_at<value_type>(item, v);
               },
               [&v](value_type* dst, value_type* src, length_type count) {
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(dst, src, sizeof(T) * (size_t)count);
                        return;
                     }
                  }
                  for (length_type i = 0; i < count; ++i)
                     std::construct_at<value_type>(&dst[i], std::move(src[i]));
               }
            );
         }

         template<typename Functor> constexpr void shift_by(coordinate_type x_delta, coordinate_type y_delta, Functor&& destroy_functor) {
            length_type max_zx = length();
            length_type max_zy = length();
            if (x_delta < 0)
               max_zx += x_delta;
            if (y_delta < 0)
               max_zy += y_delta;

            length_type zy = 0;
            for (; zy < y_delta; ++zy) { // destroy topmost items if shifting down
               for (length_type zx = 0; zx < length(); ++zx) {
                  destroy_functor(_zero_based_item(zx, zy), (coordinate_type)zx + left(), (coordinate_type)zy + top());
               }
            }
            for (; zy < max_zy; ++zy) {
               length_type zx = 0;
               for (; zx < x_delta; ++zx) { // destroy leftmost items if shifting right
                  destroy_functor(_zero_based_item(zx, zy), (coordinate_type)zx + left(), (coordinate_type)zy + top());
               }
               for (; zx < max_zx; ++zx) {
                  _zero_based_item(zx, zy) = _zero_based_item(zx, zy - y_delta);
               }
               for (; zx < length(); ++zx) { // destroy rightmost items if shifting left
                  destroy_functor(_zero_based_item(zx, zy), (coordinate_type)zx + left(), (coordinate_type)zy + top());
               }
            }
            for (; zy < length(); ++zy) { // destroy bottommost items if shifting up
               for (length_type zx = 0; zx < length(); ++zx) {
                  destroy_functor(_zero_based_item(zx, zy), (coordinate_type)zx + left(), (coordinate_type)zy + top());
               }
            }
         }
   };
}
