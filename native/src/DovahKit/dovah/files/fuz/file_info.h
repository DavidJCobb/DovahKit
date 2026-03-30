#pragma once
#include <cstdint>

namespace dovah::fuz {
   constexpr const size_t header_size = 0xC;

   struct file_info {
      constexpr file_info() {}
      file_info(const void* data, size_t size) {
         if (!data_is_fuz(data, size))
            return;
         this->version     = ((uint32_t*)data)[1];
         this->buffer.size = ((uint32_t*)data)[2];
         this->buffer.data = (const void*)((const uint8_t*)data + header_size);
      }

      static bool data_is_fuz(const void* data, size_t size) {
         if (size < header_size)
            return false;
         uint32_t signature = *(uint32_t*)data;
         if (signature != 'FUZE' && signature != 'EZUF')
            return false;
         return true;
      }

      uint32_t version = 0;
      struct {
         const void* data = nullptr;
         uint32_t    size = 0;
      } buffer;
   };
}