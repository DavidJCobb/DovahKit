#pragma once
#include <cstddef>
#include <cstdint>

namespace cobb {
   class generic_buffer {
      private:
         void*    _data = nullptr;
         uint32_t _size = 0;
         uint32_t _capacity = 0;
      public:
         void allocate(uint32_t bytes);
         void free(); // checks whether (data) is nullptr
         inline void* raw() { return this->_data; }
         void shrink_to_fit();

         inline uint32_t size() { return this->_size; }
         inline uint32_t capacity() { return this->_capacity; }

         inline void* operator->() { return this->_data; }
         inline void* operator*() { return this->_data; }

         inline operator char*() { return (char*)this->_data; }
         inline operator void*() { return this->_data; }
         inline operator bool() { return this->_data != nullptr; }
         explicit inline operator std::ptrdiff_t() { return (std::ptrdiff_t)this->_data; }

         generic_buffer() {};
         generic_buffer(uint32_t bytes) { this->allocate(bytes); }
         ~generic_buffer() {
            this->free();
         }
   };
}