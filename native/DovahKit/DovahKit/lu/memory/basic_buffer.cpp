#include "./basic_buffer.h"
#include <cstdlib> // free, malloc
#include <cstring> // memcpy
#include <stdexcept> // std::bad_alloc

namespace lu::memory {
   basic_buffer::~basic_buffer() {
      this->clear();
   }

   #pragma region Copy/move
      basic_buffer::basic_buffer(const basic_buffer& other) {
         const size_t size = other._size;
         if (size) {
            this->resize(size);
            #pragma warning(suppress: 6387, justification: "`this->_data` can't be null here; if `resize` fails, it throws")
            memcpy(this->_data, other._data, size);
         }
      }
      basic_buffer& basic_buffer::operator=(const basic_buffer& other) {
         const size_t size = other._size;
         if (!size) {
            this->clear();
         } else {
            this->resize(size);
            memcpy(this->_data, other._data, size);
         }
         return *this;
      }

      basic_buffer::basic_buffer(basic_buffer&& other) noexcept {
         this->_data = other._data;
         this->_size = other._size;
         other._data = nullptr;
         other._size = 0;
      }
      basic_buffer& basic_buffer::operator=(basic_buffer&& other) noexcept {
         this->clear();
         this->_data = other._data;
         this->_size = other._size;
         other._data = nullptr;
         other._size = 0;
         return *this;
      }
   #pragma endregion

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