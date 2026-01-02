#include "texture.h"
#include <cassert>
#include "helpers/generic_reader_ex.h"
#include "load_exception.h"

namespace {
   using dds_magic_number_type = uint32_t;
}

namespace vulkanDK::dds {
   bool texture::read() {
      if (!this->data)
         return false;
      if (this->size < header::serialized_size)
         return false;
      auto reader = cobb::generic_reader_ex(this->data, this->size);
      reader.set_endianness(std::endian::little);
      try {
         {
            dds_magic_number_type magic;
            reader.read<std::endian::big>(magic);
            if (magic != 'DDS ')
               return false;
         }
         reader.read(this->metadata);
      } catch (cobb::generic_reader_ex::unexpected_end& e) {
         return false;
      } catch (load_exception& e) {
         return false;
      }
      //
      return true;
   }

   const void* texture::pixel_data() const {
      if (!this->data)
         return nullptr;
      std::intptr_t addr = (std::intptr_t)this->data;
      addr += sizeof(dds_magic_number_type) + header::serialized_size;
      if (this->metadata.has_extended_header()) {
         addr += header_extension::serialized_size;
      }
      return (const void*)addr;
   }
   size_t texture::pixel_data_size() const {
      if (!this->data)
         return 0;
      auto s = this->size - (sizeof(dds_magic_number_type) + header::serialized_size);
      if (this->metadata.has_extended_header()) {
         s -= header_extension::serialized_size;
      }
      return s;
   }

   texture::~texture() {
      if (this->data) {
         delete this->data;
         this->data = nullptr;
      }
      this->size = 0;
   }
   texture::texture(texture&& other) noexcept {
      *this = std::move(other);
   }
   texture& texture::operator=(texture&& other) noexcept {
      std::swap(this->data, other.data);
      std::swap(this->size, other.size);
      std::swap(this->metadata, other.metadata);
      return *this;
   }

   /*static*/ texture texture::from_r8g8b8a8(const uint8_t* data, size_t size_in_bytes, uint32_t width, uint32_t height) {
      texture out;
      out.size = sizeof(dds_magic_number_type) + header::serialized_size + size_in_bytes;
      uint8_t* dds_data = (uint8_t*)malloc(out.size);
      out.data = dds_data;
      assert(!!dds_data);
      memset(dds_data, 0, out.size);

      *(dds_magic_number_type*)dds_data = 'DDS ';
      *(uint32_t*)(dds_data + 0x04) = header::serialized_size;
      *(uint32_t*)(dds_data + 0x08) = header::flag::required_flags | header::flag::has_pitch;
      *(uint32_t*)(dds_data + 0x0C) = height;
      *(uint32_t*)(dds_data + 0x10) = width;
      *(uint32_t*)(dds_data + 0x14) = 4; // pitch i.e. bytes per scanline
      *(uint32_t*)(dds_data + 0x18) = 0; // depth
      *(uint32_t*)(dds_data + 0x1C) = 0; // mipmap count
      {
         uint8_t* format = dds_data + 0x4C;
         *(uint32_t*)(format + 0x00) = pixel_format::serialized_size;
         *(uint32_t*)(format + 0x04) = pixel_format::flag::has_alpha; // flags
         *(uint32_t*)(format + 0x08) = 0; // big-endian four-CC
         *(uint32_t*)(format + 0x0C) = 32; // RGB bitcount
         *(uint32_t*)(format + 0x10) = 0xFF000000; // big-endian bitmask, red
         *(uint32_t*)(format + 0x14) = 0x00FF0000; // big-endian bitmask, green
         *(uint32_t*)(format + 0x18) = 0x0000FF00; // big-endian bitmask, blue
         *(uint32_t*)(format + 0x1C) = 0x000000FF; // big-endian bitmask, alpha
         if constexpr (std::endian::native == std::endian::little) {
            for (size_t i = 0; i < 4; ++i) {
               uint32_t& mask = *(uint32_t*)(format + 0x10 + (i * 4));
               mask = std::byteswap(mask);
            }
         }
      }
      *(uint32_t*)(dds_data + 0x6C) = header::capabilities_0::is_texture;
      *(uint32_t*)(dds_data + 0x70) = 0;
      *(uint32_t*)(dds_data + 0x74) = 0;
      *(uint32_t*)(dds_data + 0x78) = 0;
      *(uint32_t*)(dds_data + 0x7C) = 0; // reserved 2
      memcpy(dds_data + sizeof(dds_magic_number_type) + header::serialized_size, data, size_in_bytes);

      return out;
   }
}