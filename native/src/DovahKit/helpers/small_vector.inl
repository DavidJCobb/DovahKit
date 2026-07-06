#pragma once
#include "small_vector.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define TEMPLATE_PARAMS template<typename T, size_t LocalCapacity, bool Contiguous>
#define CLASS_NAME small_vector<T, LocalCapacity, Contiguous>

namespace cobb {
   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_construct(value_type* dst, value_type& src) requires(_copy_insertable || _move_insertable) {
      if constexpr (_move_insertable) {
         std::construct_at(dst, std::move(src));
      } else if constexpr (_copy_insertable) {
         std::construct_at(dst, src);
      }
   };

   TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_transfer(value_type& dst, value_type& src) requires(_copy_insertable || _move_insertable) {
      if constexpr (_move_insertable) {
         dst = std::move(src);
      } else if constexpr (_copy_insertable) {
         dst = src;
      }
   };

   TEMPLATE_PARAMS
   template<typename V>
   constexpr void CLASS_NAME::_construct_local_range(size_type begin, size_type end, V v) requires (_can_construct_local_range<V>) {
      for (size_type i = begin; i < end; ++i) {
         if constexpr (std::is_same_v<V, dummy>) {
            std::construct_at(&_data.local.list[i]);
         } else {
            std::construct_at(&_data.local.list[i], v);
         }
      }
   };

   TEMPLATE_PARAMS
   template<typename V>
   constexpr void CLASS_NAME::_resize_impl(size_type s, V v) requires (_can_construct_local_range<V>) {
      if (s == _size)
         return;
      if (s > local_capacity) {
         //
         // Setting the size so that we have to use the heap:
         //
         value_type* dst = nullptr;
         if constexpr (contiguous) {
            dst = new value_type[s];
            //
            value_type* src = nullptr;
            if (_size > local_capacity) {
               for (size_type i = 0; i < _size; ++i) {
                  _transfer(dst[i], _data.heap[i]);
               }
               delete[] _data.heap;
            } else {
               for (size_type i = 0; i < _size; ++i) {
                  _transfer(dst[i], _data.local.list[i]);
                  std::destroy_at(&_data.local.list[i]);
               }
               src = _data.local.list.data();
            }
         } else {
            dst = new value_type[s - local_capacity];
            if (_size > local_capacity) {
               size_type h = _size - local_capacity;
               for (size_type i = 0; i < h; ++i) {
                  _transfer(dst[i], _data.heap[i]);
               }
               delete[] _data.heap;
            }
         }
         _data.heap = dst;
         _size = s;
         return;
      }
      //
      // Setting the size to local-only:
      //
      if constexpr (contiguous) {
         if (_size > local_capacity) {
            for (size_type i = 0; i < s; ++i) {
               _construct(&_data.local.list[i], _data.heap[i]);
            }
            delete[] _data.heap;
            _data.heap = nullptr;
         } else {
            if (_size < s) {
               _construct_local_range(_size, s, v);
            } else {
               std::destroy(_data.local.list.begin() + s, _data.local.list.begin() + _size);
            }
         }
      } else {
         if (_data.heap) {
            delete[] _data.heap;
            _data.heap = nullptr;
         }
         if (_size < s) {
            _construct_local_range(_size, s, v);
         } else {
            std::destroy(_data.local.list.begin() + s, _data.local.list.begin() + _size);
         }
      }
      _size = s;
   };
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")