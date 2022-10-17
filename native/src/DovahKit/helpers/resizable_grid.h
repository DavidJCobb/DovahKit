#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>
#include "./unreachable.h"

namespace cobb {
   namespace impl::_resizable_grid {
      struct dummy {};

      struct no_op_functor {
         template<typename T> void operator()(T&) {}
      };
   }

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
   // A resizable square grid centered on (0, 0). If the grid length is an even number, 
   // then the grid biases toward the upper-left (i.e. the center is closer to the lower-
   // right corner).
   //
   template<typename T, typename CoordinateType = int32_t> requires std::is_signed_v<CoordinateType>
   class resizable_square_grid {
      public:
         using value_type      = T;
         using coordinate_type = CoordinateType;
         using length_type     = std::make_unsigned_t<CoordinateType>;
         using size_type       = std::size_t;
         
      protected:
         static constexpr const bool _can_default = std::is_default_constructible_v<value_type>;
         static constexpr const bool _can_copy    = std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>;
         static constexpr const bool _can_move    = std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>;

         struct _no_op_destroy_functor {
            void operator()(value_type&) {}
         };
         
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

         __declspec(allocator) static constexpr value_type* _alloc(size_t count, bool skip_construct = false) {
            if (std::is_constant_evaluated()) {
               if (skip_construct)
                  throw; // not supported in constexpr
               return new value_type[count];
            } else {
               auto* result = (value_type*)malloc(sizeof(value_type) * count);
               if (!skip_construct) {
                  new (result) value_type[count];
               }
               return result;
            }
         }
         static constexpr void _free(value_type* memory, size_t count, bool skip_destruct = false) {
            if (std::is_constant_evaluated()) {
               if (skip_destruct)
                  throw; // not supported in constexpr
               delete[] memory;
            } else {
               if (!skip_destruct) {
                  for (size_t i = 0; i < count; ++i)
                     std::destroy_at<value_type>(memory + i);
               }
               free(memory);
            }
         }

         static constexpr value_type& _item_in_new_grid(value_type* buffer, length_type length, length_type x, length_type y) {
            return buffer[x + (y * length)];
         }

         template<typename Functor>
         static constexpr void _for_perimeter(value_type* buffer, length_type outer_length, length_type inner_length, Functor&& functor) {
            size_t margin_start = (outer_length - inner_length + 1) / 2; // + 1 to bias to the left
            size_t margin_end   = (outer_length - inner_length) / 2;

            for (size_t y = 0; y < margin_start; ++y) { // old top row(s), downward
               for (size_t x = 0; x < outer_length; ++x) {
                  auto& item = buffer[y * outer_length + x];
                  functor(item);
               }
            }
            for (size_t inv_y = 0; inv_y < margin_end; ++inv_y) { // old bottom row(s), upward
               size_t y = outer_length - inv_y - 1;
               for (size_t x = 0; x < outer_length; ++x) {
                  auto& item = buffer[y * outer_length + x];
                  functor(item);
               }
            }
            //
            auto y_end = outer_length - margin_end;
            for (size_t x = 0; x < margin_start; ++x) { // old left col(s), rightward
               for (size_t y = margin_start; y < y_end; ++y) {
                  auto& item = buffer[y * outer_length + x];
                  functor(item);
               }
            }
            for (size_t inv_x = 0; inv_x < margin_end; ++inv_x) { // old right col(s), leftward
               size_t x = outer_length - inv_x - 1;
               for (size_t y = margin_start; y < y_end; ++y) {
                  auto& item = buffer[y * outer_length + x];
                  functor(item);
               }
            }
         }
         //
         template<typename InitializeNewTo = impl::_resizable_grid::dummy, typename DestroyFunctor = impl::_resizable_grid::dummy>
         constexpr void _resize_impl(length_type new_length, const InitializeNewTo& values, DestroyFunctor destroy_functor = {}) {
            auto old_length = this->_length;
            if (new_length == old_length)
               return;
            if (new_length == 0) {
               this->clear_and_collapse();
               return;
            }
            this->_length = new_length;
            if (new_length > old_length) {
               //
               // Enlarging the grid.
               //
               value_type* src_buffer = this->_data;
               value_type* dst_buffer = nullptr;
               if constexpr (std::is_trivially_copyable_v<value_type>) {
                  if (std::is_constant_evaluated()) {
                     dst_buffer = _alloc(new_length * new_length);
                     this->_data = dst_buffer;
                  } else {
                     dst_buffer = this->_data = (value_type*)realloc(this->_data, sizeof(value_type) * new_length * new_length);
                  }
               } else {
                  dst_buffer = this->_data = _alloc(new_length * new_length, true);
               }
               if (!src_buffer) {
                  //
                  // The grid was empty with a zero-size buffer.
                  //
                  for (size_t i = 0; i < new_length * new_length; ++i) {
                     auto& item = dst_buffer[i];
                     if constexpr (std::is_same_v< InitializeNewTo, impl::_resizable_grid::dummy>) {
                        std::construct_at<value_type>(&item);
                     } else {
                        std::construct_at<value_type>(&item, values);
                     }
                  }
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        return; // used realloc; don't need to free old memory
                     }
                  }
                  return;
               }
               //
               // First, copy the old rows into where they need to go.
               //
               size_t prior_count  = old_length * old_length;
               size_t margin_start = (new_length - old_length + 1) / 2; // + 1 to bias to the left
               size_t margin_end   = (new_length - old_length) / 2;
               for (size_t inv_src_y = 0; inv_src_y < old_length; ++inv_src_y) { // go in reverse order
                  size_t src_y = old_length - inv_src_y - 1;
                  size_t dst_y = margin_start + src_y;
                  auto&  src   = src_buffer[src_y * old_length];
                  auto&  dst   = dst_buffer[dst_y * new_length + margin_start];
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(&dst, &src, sizeof(value_type) * old_length);
                        continue;
                     }
                  }
                  for (size_t x = 0; x < old_length; ++x) {
                     auto& s = (&src)[x];
                     auto& d = (&dst)[x];
                     std::construct_at<value_type>(&d, std::move(s));
                  }
               }
               //
               // Initialize new items:
               //
               _for_perimeter(
                  dst_buffer,
                  new_length,
                  old_length,
                  [&values](value_type& item) {
                     if constexpr (std::is_same_v< InitializeNewTo, impl::_resizable_grid::dummy>) {
                        std::construct_at<value_type>(&item);
                     } else {
                        std::construct_at<value_type>(&item, values);
                     }
                  }
               );
               //
               if (std::is_constant_evaluated()) {
                  _free(src_buffer, old_length * old_length);
               }
               return;
            }
            if (new_length < old_length) {
               value_type* src_buffer = this->_data;
               value_type* dst_buffer = _alloc(new_length * new_length, !std::is_constant_evaluated());
               this->_data = dst_buffer;
               
               size_t margin_start = (old_length - new_length + 1) / 2; // + 1 to bias to the left
               size_t margin_end   = (old_length - new_length) / 2;
               for (length_type y = 0; y < new_length; ++y) {
                  size_t src_y = margin_start + y;
                  size_t dst_y = y;
                  auto&  src   = src_buffer[src_y * old_length + margin_start];
                  auto&  dst   = dst_buffer[dst_y * new_length];
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(&dst, &src, sizeof(value_type) * new_length);
                        continue;
                     }
                  }
                  for (size_t x = 0; x < new_length; ++x) {
                     auto& s = (&src)[x];
                     auto& d = (&dst)[x];
                     std::construct_at<value_type>(&d, std::move(s));
                  }
               }
               //
               // Destroy old items:
               //
               _for_perimeter(
                  src_buffer,
                  old_length,
                  new_length,
                  [&destroy_functor](value_type& item) {
                     if constexpr (!std::is_same_v< DestroyFunctor, impl::_resizable_grid::dummy>) {
                        destroy_functor(item);
                     }
                     if (!std::is_constant_evaluated()) {
                        std::destroy_at(&item);
                     }
                  }
               );
               //
               _free(src_buffer, old_length * old_length);
               return;
            }
            cobb::unreachable();
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
         constexpr value_type& from_corner(length_type x, length_type y) noexcept {
            return this->data()[x + (y * this->length())];
         }
         constexpr const value_type& from_corner(length_type x, length_type y) const noexcept {
            return this->data()[x + (y * this->length())];
         }
         
         iterator begin() { return iterator{ _data }; }
         iterator end() { return iterator{ _data + area() }; }
         const_iterator cbegin() const { return const_iterator{ _data }; }
         const_iterator cend() const { return const_iterator{ _data + area() }; }
         const_iterator begin() const { return cbegin(); }
         const_iterator end() const { return cend(); }

         template<typename Functor> void for_each(Functor&& functor) {
            for (coordinate_type y = top(); y <= bottom(); ++y)
               for (coordinate_type x = left(); x <= right(); ++x)
                  functor(at(x, y), x, y);
         }
         template<typename Functor> void for_each(Functor&& functor) const {
            for (coordinate_type y = top(); y <= bottom(); ++y)
               for (coordinate_type x = left(); x <= right(); ++x)
                  functor(at(x, y), x, y);
         }

         constexpr void clear() {
            for (size_t i = 0; i < this->area(); ++i)
               this->_data[i] = value_type{};
         }
         constexpr void clear_and_collapse() {
            if (this->_data) {
               _free(this->_data, this->_length * this->_length);
               this->_data = nullptr;
            }
            this->_length = 0;
         }

         constexpr void resize(length_type new_length) requires (_can_default || _can_move) {
            this->_resize_impl(
               new_length,
               impl::_resizable_grid::dummy{},
               [](value_type& to_destroy) {}
            );
         }

         template<typename DestroyFunctor = _no_op_destroy_functor> requires (!std::is_same_v<DestroyFunctor, value_type>)
         constexpr void resize(length_type new_length, const DestroyFunctor& destroy = {})
            requires (_can_default || _can_move)
         {
            this->_resize_impl(
               new_length,
               impl::_resizable_grid::dummy{},
               [&destroy](value_type& to_destroy) {
                  destroy(to_destroy);
               }
            );
         }

         template<typename DestroyFunctor = _no_op_destroy_functor> requires (!std::is_same_v<DestroyFunctor, value_type>)
         constexpr void resize(length_type new_length, const value_type& v, const DestroyFunctor& destroy = {})
            requires (_can_copy && _can_move)
         {
            this->_resize_impl(
               new_length,
               v,
               [&destroy](value_type& to_destroy) {
                  destroy(to_destroy);
               }
            );
         }

         template<typename Functor> constexpr void shift_by(coordinate_type x_delta, coordinate_type y_delta, Functor&& destroy_functor) {
            auto x_d_abs = abs(x_delta);
            auto y_d_abs = abs(y_delta);
            if (x_d_abs > this->length() || y_d_abs > this->length()) {
               //
               // We're shifting by a large enough magnitude to clear the grid entirely.
               //
               this->for_each(destroy_functor);
               this->clear();
               return;
            }
            //
            const auto length = this->length();
            const auto lookup = [length](value_type* data, coordinate_type x, coordinate_type y) -> value_type* {
               if (x < 0 || x >= length)
                  return nullptr;
               if (y < 0 || y >= length)
                  return nullptr;
               return &data[x + (y * length)];
            };

            auto* working = _alloc(length * length);
            for (length_type y = 0; y < length; ++y) {
               for (length_type x = 0; x < length; ++x) {
                  auto& src = from_corner(x, y);
                  auto* dst = lookup(working, x + x_delta, y + y_delta);
                  if (dst) {
                     *dst = std::move(src);
                  } else {
                     destroy_functor(src, (coordinate_type)x - x_delta + left(), (coordinate_type)y - y_delta + top());
                  }
               }
            }
            _free(this->_data, this->_length * this->_length);
            this->_data = working;
         }
   };
}
