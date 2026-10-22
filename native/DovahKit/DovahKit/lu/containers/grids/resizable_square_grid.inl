#pragma once
#include "./resizable_square_grid.h"
#include <cassert>
#include "../../foreach_lambda_result.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T, typename CoordinateType> requires std::is_signed_v<CoordinateType>
#define CLASS_NAME resizable_square_grid<T, CoordinateType>

namespace lu {
   #pragma region Internal alloc/free
      TEMPLATE_PARAMS
      #ifdef _MSC_VER
         __declspec(allocator)
      #endif
      /*static*/ constexpr CLASS_NAME::value_type* CLASS_NAME::_alloc(size_t count, bool skip_construct) {
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

      TEMPLATE_PARAMS
      /*static*/ constexpr void CLASS_NAME::_free(value_type* memory, size_t count, bool skip_destruct) {
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
   #pragma endregion

   TEMPLATE_PARAMS
   template<typename Functor>
   /*static*/ constexpr void CLASS_NAME::_for_perimeter(value_type* buffer, length_type outer_length, length_type inner_length, Functor&& functor) {
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

   TEMPLATE_PARAMS
   template<typename InitializeNewTo, typename DestroyFunctor>
   constexpr void CLASS_NAME::_resize_impl(length_type new_length, const InitializeNewTo& values, DestroyFunctor destroy_functor) {
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
               if constexpr (std::is_same_v<InitializeNewTo, impl::_resizable_grid::dummy>) {
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
               if constexpr (std::is_same_v<InitializeNewTo, impl::_resizable_grid::dummy>) {
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
               if constexpr (!std::is_same_v<DestroyFunctor, impl::_resizable_grid::dummy>) {
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
      std::unreachable();
   }

   #pragma region Constructor, destructor, and assignment operators
      TEMPLATE_PARAMS
      constexpr CLASS_NAME& CLASS_NAME::operator=(const CLASS_NAME& o) requires (_can_copy) {
         this->clear_and_collapse();
         if (!o.empty()) {
            if constexpr (_can_default) {
               this->resize(o.length());
               for (size_t i = 0; i < area(); ++i) {
                  this->data()[i] = o.data()[i];
               }
            } else {
               const size_t area = o.area();
               this->_data = _alloc(area);
               for (size_t i = 0; i < area; ++i) {
                  const value_type& src = o.item_by_flat_index(i);
                  value_type&       dst = this->_data[i];
                  if constexpr (std::is_trivially_copyable_v<value_type>) {
                     if (!std::is_constant_evaluated()) {
                        memcpy(&dst, &src, sizeof(value_type) * old_length);
                        continue;
                     }
                  }
                  std::construct_at<value_type>(dst, src);
               }
            }
         }
         return *this;
      }
   #pragma endregion

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear() requires (_can_default) {
      for (size_t i = 0; i < this->area(); ++i)
         this->_data[i] = value_type{};
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear_and_collapse() {
      if (this->_data) {
         _free(this->_data, this->_length * this->_length);
         this->_data = nullptr;
      }
      this->_length = 0;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::contains_coordinate(coordinate_type x, coordinate_type y) const noexcept {
      if (x < left() || x > right())
         return false;
      if (y < top() || y > bottom())
         return false;
      return true;
   }

   #pragma region for_each
      TEMPLATE_PARAMS
      template<typename Functor>
      void CLASS_NAME::for_each(Functor&& functor) const
         requires (
            std::is_invocable_v<Functor, const value_type&, coordinate_type, coordinate_type> ||
            std::is_invocable_v<Functor, const value_type&>
         )
      {
         for (coordinate_type y = top(); y <= bottom(); ++y) {
            for (coordinate_type x = left(); x <= right(); ++x) {
               auto& value = at(x, y);
               if constexpr (std::is_invocable_r_v<foreach_lambda_result, Functor, const value_type&, coordinate_type, coordinate_type>) {
                  if (functor(value, x, y) == foreach_lambda_result::stop)
                     return;
               } else if constexpr (std::is_invocable_v<Functor, const value_type&, coordinate_type, coordinate_type>) {
                  functor(value, x, y);
               } else if constexpr (std::is_invocable_r_v<foreach_lambda_result, Functor, const value_type&>) {
                  if (functor(value) == foreach_lambda_result::stop)
                     return;
               } else {
                  functor(value);
               }
            }
         }
      }
      
      TEMPLATE_PARAMS
      template<typename Functor>
      void CLASS_NAME::for_each(Functor&& functor)
         requires (
            std::is_invocable_v<Functor, value_type&, coordinate_type, coordinate_type> ||
            std::is_invocable_v<Functor, value_type&>
         )
      {
         for (coordinate_type y = top(); y <= bottom(); ++y) {
            for (coordinate_type x = left(); x <= right(); ++x) {
               auto& value = at(x, y);
               if constexpr (std::is_invocable_r_v<foreach_lambda_result, Functor, value_type&, coordinate_type, coordinate_type>) {
                  if (functor(value, x, y) == foreach_lambda_result::stop)
                     return;
               } else if constexpr (std::is_invocable_v<Functor, value_type&, coordinate_type, coordinate_type>) {
                  functor(value, x, y);
               } else if constexpr (std::is_invocable_r_v<foreach_lambda_result, Functor, value_type&>) {
                  if (functor(value) == foreach_lambda_result::stop)
                     return;
               } else {
                  functor(value);
               }
            }
         }
      }
   #pragma endregion

   #pragma region item
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item(coordinate_type x, coordinate_type y) const noexcept {
         const auto half = this->length() / 2;
         return this->item_from_corner(x + half, y + half);
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::item(coordinate_type x, coordinate_type y) noexcept {
         const auto half = this->length() / 2;
         return this->item_from_corner(x + half, y + half);
      }
   #pragma endregion

   #pragma region item_by_flat_index
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index(size_t i) const noexcept {
         assert(i < this->area());
         return this->data()[i];
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::item_by_flat_index(size_t i) noexcept {
         assert(i < this->area());
         return this->data()[i];
      }
   #pragma endregion

   #pragma region item_from_corner
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::item_from_corner(length_type x, length_type y) const noexcept {
         return this->data()[x + (y * this->length())];
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::item_from_corner(length_type x, length_type y) noexcept {
         return this->data()[x + (y * this->length())];
      }
   #pragma endregion

   #pragma region resize
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::resize(length_type new_length) requires (_can_default || _can_move) {
         this->_resize_impl(
            new_length,
            impl::_resizable_grid::dummy{},
            [](value_type& to_destroy) {}
         );
      }

      TEMPLATE_PARAMS
      template<typename DestroyFunctor>
      constexpr void CLASS_NAME::resize(length_type new_length, const DestroyFunctor& destroy)
         requires (
            (_can_default || _can_move) &&
            !std::is_same_v<DestroyFunctor, value_type> &&
            std::is_invocable_v<DestroyFunctor, value_type&>
         )
      {
         this->_resize_impl(
            new_length,
            impl::_resizable_grid::dummy{},
            [&destroy](value_type& to_destroy) {
               destroy(to_destroy);
            }
         );
      }

      TEMPLATE_PARAMS
      template<typename DestroyFunctor>
      constexpr void CLASS_NAME::resize(length_type new_length, const value_type& v, const DestroyFunctor& destroy)
         requires (
            (_can_copy && _can_move) &&
            std::is_invocable_v<DestroyFunctor, value_type&>
         )
      {
         this->_resize_impl(
            new_length,
            v,
            [&destroy](value_type& to_destroy) {
               destroy(to_destroy);
            }
         );
      }
   #pragma endregion

   TEMPLATE_PARAMS
   template<typename Functor>
   constexpr void CLASS_NAME::shift_by(coordinate_type x_delta, coordinate_type y_delta, Functor&& destroy_functor) {
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
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")