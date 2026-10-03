#pragma once
#include "./buffer_view.h"

namespace lu::memory {
   constexpr buffer_view::buffer_view(const void* data, size_type size) {
      this->_data = data;
      if (data)
         this->_size = size;
      else
         this->_size = 0;
   }

   constexpr const void* buffer_view::data() const noexcept {
      return this->_data;
   }
   constexpr const void* buffer_view::data_at(size_type offset) const noexcept {
      return (const void*)((const char*)this->_data + offset);
   }
   constexpr bool buffer_view::empty() const noexcept {
      return this->_size == 0;
   }
   constexpr const void* buffer_view::end() const noexcept {
      return this->data_at(this->_size);
   }
   constexpr buffer_view::size_type buffer_view::size() const noexcept {
      return this->_size;
   }
}