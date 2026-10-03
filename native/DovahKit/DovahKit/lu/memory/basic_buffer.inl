#pragma once
#include "./basic_buffer.h"

namespace lu::memory {
   constexpr const void* basic_buffer::data() const noexcept {
      return this->_data;
   }
   constexpr void* basic_buffer::data() noexcept {
      return this->_data;
   }
   constexpr bool basic_buffer::empty() const noexcept {
      return this->_size == 0;
   }
   constexpr size_t basic_buffer::size() const noexcept {
      return this->_size;
   }

   constexpr const void* basic_buffer::data_at(size_t offset) const noexcept {
      return (const void*)((const char*)this->_data + offset);
   }
   constexpr void* basic_buffer::data_at(size_t offset) noexcept {
      return (void*)((const char*)this->_data + offset);
   }
}