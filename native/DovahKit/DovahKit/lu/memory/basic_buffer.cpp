#include "./basic_buffer.h"
#include <cstdlib>
#include <stdexcept>

namespace lu::memory {
   basic_buffer::~basic_buffer() {
      this->clear();
   }

   void basic_buffer::clear() {
      if (this->_data) {
         free(this->_data);
         this->_data = nullptr;
      }
      this->_size = 0;
   }

   void basic_buffer::resize(size_t size) {
      if (this->_size == size)
         return;
      if (size == 0) {
         this->clear();
         return;
      }
      if (!this->_data) {
         this->_data = malloc(size);
         if (!this->_data)
            throw std::bad_alloc();
         return;
      }
      auto* moved = realloc(this->_data, size);
      if (!moved)
         throw std::bad_alloc();
      this->_data = moved;
   }
}