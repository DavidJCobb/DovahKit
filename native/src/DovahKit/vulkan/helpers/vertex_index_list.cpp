#include "vertex_index_list.h"
#include <algorithm>
#include <cassert>
#include <stdlib.h>
//
#include <intrin.h>
#include "../../helpers/cpuinfo.h"

namespace {
   uint32_t* _convert(uint16_t* list, size_t size, size_t capacity) {
      auto*  out = (uint32_t*) malloc(sizeof(uint32_t) * capacity);
      size_t i   = 0;
      //
      bool can_intrin = false;
      {
         auto& c = cobb::cpuinfo::get().extension_support;
         can_intrin = c.sse_2 && c.sse_4_1;
      }
      if (can_intrin) {
         for (; i + 7 < size; i += 8) {
            auto here = (std::intptr_t)&out[i];
            auto half = (std::intptr_t)&out[i] + (sizeof(uint32_t) * 4);

            __m128i src = _mm_loadu_si128((__m128i*)&list[i]);
            __m128i dst = _mm_cvtepu16_epi32(src); // convert first four uint16_ts
            _mm_storeu_si128((__m128i*)here, dst);
            src = _mm_srli_si128(src, 8);  // shift by 8 bytes
            dst = _mm_cvtepu16_epi32(src); // convert second four uint16_ts
            _mm_storeu_si128((__m128i*)half, dst);
         }
      }
      for (; i < size; ++i) {
         out[i] = list[i];
      }
      return out;
   }
   inline uint32_t* _convert(uint16_t* list, size_t size) {
      return _convert(list, size, size);
   }
   //
   inline uint16_t* _convert(uint32_t* list, size_t size, size_t capacity) {
      auto*  out = (uint16_t*) malloc(sizeof(uint16_t) * capacity);
      size_t i   = 0;
      for (; i < size; ++i) {
         out[i] = list[i];
      }
      return out;
   }
   inline uint16_t* _convert(uint32_t* list, size_t size) {
      return _convert(list, size, size);
   }

   template<typename T> T* _reserve(const T* list, size_t size, size_t capacity) {
      if (capacity == 0)
         return nullptr;
      auto* out = (T*) malloc(sizeof(T) * capacity);
      if (list) {
         memcpy(out, list, sizeof(T) * size);
      } else {
         assert(!size);
      }
      #if _DEBUG
         if (capacity > size) {
            std::intptr_t at = (std::intptr_t)out + (sizeof(T) * size);
            memset((void*)at, 0, sizeof(T) * (capacity - size));
         }
      #endif
      return out;
   }

   template<typename T> T* _resize(const T* list, size_t prior, size_t after) {
      if (after == 0)
         return nullptr;
      size_t in_bytes = sizeof(T) * after;
      auto*  out      = (T*) malloc(in_bytes);
      //
      size_t count = std::min(prior, after);
      if (prior) {
         assert(list);
         memcpy(out, list, sizeof(T) * count);
      }
      if (prior < after) {
         std::intptr_t at = (std::intptr_t)out + (sizeof(T) * prior);
         memset((void*)at, 0, sizeof(T) * (after - prior));
      }
      return out;
   }
}

namespace vulkanDK {
   vertex_index_list::~vertex_index_list() {
      this->clear();
   }

   vertex_index_list::vertex_index_list(size_t count, uint16_t value) {
      this->_type      = value_type::thin;
      this->_size      = count;
      this->_capacity  = count;
      this->_data.thin = (decltype(value)*) malloc(count * sizeof(decltype(value)));
      //
      if (value == 0) {
         memset(this->_data.untyped, 0, count * sizeof(decltype(value)));
      } else {
         for (size_t i = 0; i < count; ++i)
            this->_data.thin[i] = value;
      }
   }
   vertex_index_list::vertex_index_list(size_t count, uint32_t value) {
      this->_type      = value_type::thin;
      this->_size      = count;
      this->_capacity  = count;
      this->_data.wide = (decltype(value)*) malloc(count * sizeof(decltype(value)));
      //
      if (value == 0) {
         memset(this->_data.untyped, 0, count * sizeof(decltype(value)));
      } else {
         for (size_t i = 0; i < count; ++i)
            this->_data.wide[i] = value;
      }
   }

   vertex_index_list::vertex_index_list(const vertex_index_list& o) {
      this->_size = o._size;
      this->_type = o._type;
      if (o._type == value_type::thin) {
         this->_data.thin = _reserve(o._data.thin, this->_size, this->_size);
      } else {
         this->_data.wide = _reserve(o._data.wide, this->_size, this->_size);
      }
      this->_capacity = this->_size;
   }
   vertex_index_list& vertex_index_list::operator=(const vertex_index_list& o) {
      if (this->_type == o._type) {
         if (this->_size >= o._size) {
            memcpy(this->_data.untyped, o._data.untyped, o.size_in_bytes());
            this->_size = o._size;
            return *this;
         }
      }
      //
      // Couldn't reuse our existing storage space.
      //
      if (this->_data.untyped) {
         delete this->_data.untyped;
         #if _DEBUG
            this->_data.untyped = nullptr;
         #endif
      }
      this->_size = o._size;
      if (o._type == value_type::thin) {
         this->_data.thin = _reserve(o._data.thin, this->_size, this->_size);
      } else {
         this->_data.wide = _reserve(o._data.wide, this->_size, this->_size);
      }
      this->_capacity = this->_size;
      //
      return *this;
   }

   vertex_index_list::vertex_index_list(vertex_index_list&& o) noexcept {
      *this = std::move(o);
   }
   vertex_index_list& vertex_index_list::operator=(vertex_index_list&& o) noexcept {
      std::swap(this->_type,     o._type);
      std::swap(this->_data,     o._data);
      std::swap(this->_size,     o._size);
      std::swap(this->_capacity, o._capacity);
      return *this;
   }

   void vertex_index_list::clear() {
      this->_size     = 0;
      this->_capacity = 0;
      if (this->_data.untyped) {
         delete this->_data.untyped;
         this->_data.untyped = nullptr;
      }
   }
   void vertex_index_list::set_type(value_type t) {
      auto prior = this->_type;
      if (prior == t)
         return;
      this->_type = t;
      if (this->_size == 0) {
         if (this->_data.untyped) {
            delete this->_data.untyped;
            this->_data.untyped = nullptr;
            this->_capacity = 0;
         }
         return;
      }
      assert(this->_data.untyped);
      if (t == value_type::thin) {
         //
         // Wide to thin:
         //
         auto* old = this->_data.wide;
         auto* now = _convert(old, this->_size);
         this->_data.thin = now;
         this->_capacity  = this->_size;
         if (old)
            delete old;
      } else {
         //
         // Thin to wide:
         //
         auto* old = this->_data.thin;
         auto* now = _convert(old, this->_size);
         this->_data.wide = now;
         this->_capacity  = this->_size;
         if (old)
            delete old;
      }
   }

   void vertex_index_list::reserve(size_t to) {
      if (to <= this->_capacity)
         return;
      if (this->_type == value_type::thin) {
         auto& list = this->_data.thin;
         //
         const auto* old = list;
         auto* now = _reserve(list, this->_size, to);
         list = now;
         if (old)
            delete old;
      } else {
         auto& list = this->_data.wide;
         //
         const auto* old = list;
         auto* now = _reserve(list, this->_size, to);
         list = now;
         if (old)
            delete old;
      }
      this->_capacity = to;
   }
   void vertex_index_list::resize(size_t to) {
      if (to == this->_size)
         return;
      if (this->_type == value_type::thin) {
         auto& list = this->_data.thin;
         //
         const auto* old = list;
         auto* now = _resize(old, this->_size, to);
         list            = now;
         this->_size     = to;
         this->_capacity = to;
         if (old)
            delete old;
      } else {
         auto& list = this->_data.wide;
         //
         const auto* old = list;
         auto* now = _resize(old, this->_size, to);
         list            = now;
         this->_size     = to;
         this->_capacity = to;
         if (old)
            delete old;
      }
   }

   uint16_t* vertex_index_list::thin_data() const {
      if (this->_type == value_type::thin)
         return this->_data.thin;
      return nullptr;
   }
   uint32_t* vertex_index_list::wide_data() const {
      if (this->_type == value_type::wide)
         return this->_data.wide;
      return nullptr;
   }
}