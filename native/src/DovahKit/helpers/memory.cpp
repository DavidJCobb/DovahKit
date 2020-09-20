#include "memory.h"
#include <cstdlib>

namespace cobb {
   void generic_buffer::clear() {
      if (this->_data) {
         free(this->_data);
         this->_data = nullptr;
         this->_size     = 0;
         this->_capacity = 0;
      }
   }
   void generic_buffer::shrink_to_fit() {
      if (this->_size < this->_capacity) {
         auto buf = realloc(this->_data, this->_size);
         if (buf) {
            this->_data     = buf;
            this->_capacity = this->_size;
         }
      }
   }
   void generic_buffer::resize(uint32_t bytes) {
      if (bytes == 0) {
         free(this->_data);
         this->_data     = nullptr;
         this->_size     = 0;
         this->_capacity = 0;
         return;
      }
      this->reserve(bytes);
      #if _DEBUG
         assert(this->_capacity >= bytes);
      #endif
      if (this->_capacity >= bytes) // check, in case the process didn't have enough memory to perform the reservation
         this->_size = bytes;
   }
   void generic_buffer::reserve(uint32_t bytes) {
      if (bytes <= this->_capacity)
         return;
      auto buffer = realloc(this->_data, bytes);
      if (!buffer) { // if allocation failed (typically due to the process not having (bytes) much memory to spare)
         return;
      }
      this->_data     = buffer;
      this->_capacity = bytes;
   }
   generic_buffer& generic_buffer::operator=(const generic_buffer& other) noexcept {
      auto size = other.size();
      this->resize(size);
      memcpy(this->_data, other.data(), size);
      return *this;
   }
   generic_buffer& generic_buffer::operator=(generic_buffer&& other) noexcept {
      this->clear();
      this->_data     = other._data;
      this->_size     = other._size;
      this->_capacity = other._capacity;
      other._data     = nullptr;
      other._size     = 0;
      other._capacity = 0;
      return *this;
   }
}