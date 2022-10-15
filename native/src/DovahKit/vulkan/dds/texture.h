#pragma once
#include "header.h"

namespace vulkanDK::dds {
   struct texture {
      public:
         const void* data = nullptr;
         size_t      size = 0;

         header metadata;

         bool read();

         const void* pixel_data() const;
         size_t pixel_data_size() const;

         texture() = default;
         ~texture();
         texture(const texture&) = default;
         texture(texture&&) noexcept;
         texture& operator=(const texture&) = default;
         texture& operator=(texture&&) noexcept;
   };
}