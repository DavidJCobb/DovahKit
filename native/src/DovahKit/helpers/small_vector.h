#pragma once
#include <array>
#include <memory>
#include <type_traits>

namespace cobb {
   template<typename T, size_t LocalCapacity, bool Contiguous = false>
   class small_vector {
      public:
         using value_type = T;
         using size_type  = size_t;
         static constexpr const size_type local_capacity = LocalCapacity;
         static constexpr const bool contiguous = Contiguous;

         using reference = value_type&;
         using const_reference = const value_type&;

      protected:
         static constexpr const bool _default_insertable = std::is_default_constructible_v<value_type>;
         static constexpr const bool _copy_insertable = std::is_copy_assignable_v<value_type> || std::is_copy_constructible_v<value_type>;
         static constexpr const bool _move_insertable = std::is_move_assignable_v<value_type> || std::is_move_constructible_v<value_type>;

         struct dummy {};

         constexpr void _construct(value_type* dst, value_type& src) requires(_copy_insertable || _move_insertable) {
            if constexpr (_move_insertable) {
               std::construct_at(dst, std::move(src));
            } else if constexpr (_copy_insertable) {
               std::construct_at(dst, src);
            }
         }
         constexpr void _transfer(value_type& dst, value_type& src) requires(_copy_insertable || _move_insertable) {
            if constexpr (_move_insertable) {
               dst = std::move(src);
            } else if constexpr (_copy_insertable) {
               dst = src;
            }
         }

         template<typename V = dummy> requires ((_default_insertable&& std::is_same_v<V, dummy>) || std::is_same_v<V, const value_type&>)
         constexpr void _construct_local_range(size_type begin, size_type end, V v) {
            for (size_type i = begin; i < end; ++i) {
               if (std::is_same_v<V, dummy>) {
                  std::construct_at(&_data.local[i]);
               } else {
                  std::construct_at(&_data.local[i], v);
               }
            }
         }

         template<typename V = dummy> requires ((_default_insertable && std::is_same_v<V, dummy>) || std::is_same_v<V, const value_type&>)
         constexpr void _resize_impl(size_type s, V v) {
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
                        _transfer(dst[i], _data.local[i]);
                        std::destroy(&_data.local[i]);
                     }
                     src = _data.local.data();
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
                     _construct(&_data.local[i], _data.heap[i]);
                  }
                  delete[] _data.heap;
                  _data.heap = nullptr;
               } else {
                  if (_size < s) {
                     _construct_local_range(_size, s, v);
                  } else {
                     std::destroy(_data.local.begin() + s, _data.local.begin() + _size);
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
                  std::destroy(_data.local.begin() + s, _data.local.begin() + _size);
               }
            }
            _size = s;
         }

      protected:
         struct {
            union {
               std::array<value_type, LocalCapacity> local;
               bool _this_union_and_bool_prevent_local_auto_construct_destruct = {};
            };
            value_type* heap = nullptr;
         } _data;
         size_t _size = 0;
         
      public:
         small_vector() {}
         ~small_vector() {
            clear();
         }
            
         const_reference operator[](size_type i) const {
            if constexpr (contiguous) {
               if (_size > local_capacity)
                  return _data.heap[i];
            } else {
               if (i >= local_capacity)
                  return _data.heap[i - local_capacity];
            }
            return _data.local[i];
         }

         inline constexpr bool empty() const noexcept { return _size == 0; }
         inline constexpr size_type size() const noexcept { return _size; }

         constexpr void resize(size_type s) requires (_default_insertable) {
            _resize_impl(s, dummy{});
         }
         constexpr void resize(size_type s, const value_type& v) requires (_copy_insertable) {
            _resize_impl(s, v);
         }

         void clear() {
            if constexpr (contiguous) {
               if (_size <= local_capacity) {
                  std::destroy(_data.local.begin(), _data.local.begin() + _size);
               }
            } else {
               std::destroy(_data.local.begin(), _data.local.begin() + _size);
            }
            _size = 0;
            if (_data.heap) {
               delete[] _data.heap;
               _data.heap = nullptr;
            }
         }

         template<typename... Args>
         reference emplace_back(Args&&... args) {
            auto i = _size;
            resize(_size + 1);
            //
            auto& item = operator[](i);
            std::construct_at(&item, std::forward<Args>(args)...);
            return item;
         }

         void push_back(const value_type& v) {
            auto i = _size;
            resize(_size + 1);
            operator[](i) = v;
         }
         void push_back(value_type&& v) {
            auto i = _size;
            resize(_size + 1);
            operator[](i) = std::move(v);
         }
   };
}