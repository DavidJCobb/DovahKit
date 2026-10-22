#pragma once
#include "./resizable_grid.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T, typename CoordinateType> requires std::is_signed_v<CoordinateType>
#define CLASS_NAME resizable_grid<T, CoordinateType>

namespace lu {
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_resize_impl(length_type new_width, length_type new_height, auto NewStripFunctor, auto PreservedStripFunctor) {
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

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear() {
      for (size_t i = 0; i < this->area(); ++i)
         this->_data[i] = value_type{};
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::clear_and_collapse() {
      if (this->_data) {
         _free(this->_data);
         this->_data = nullptr;
      }
      this->_w = 0;
      this->_h = 0;
   }

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::resize(length_type new_width, length_type new_height)
      requires (std::is_default_constructible_v<value_type>&& std::is_move_constructible_v<value_type>)
   {
      this->_resize_impl(
         new_width,
         new_height,
         [](value_type* base, size_t count) {
            for (size_t i = 0; i < count; ++i)
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

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::resize(length_type new_width, length_type new_height, const value_type& v)
      requires (std::is_copy_constructible_v<value_type> && std::is_move_constructible_v<value_type>)
   {
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
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")