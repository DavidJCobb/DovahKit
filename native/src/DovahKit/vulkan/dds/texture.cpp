#include "texture.h"
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
}