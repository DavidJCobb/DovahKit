#include "header.h"
#include "helpers/generic_reader_ex.h"
#include "load_exception.h"

namespace vulkanDK::dds {
   void header_extension::read(cobb::generic_reader_ex& reader) {
      reader.require_size(serialized_size);
      reader.unchecked_read(this->format);
      reader.unchecked_read(this->dimension);
      reader.unchecked_read(this->misc_flags_a);
      reader.unchecked_read(this->array_size);
      reader.unchecked_read(this->misc_flags_b);
   }

   void header::read(cobb::generic_reader_ex& reader) {
      reader.read(this->size);
      if (this->size != header::serialized_size)
         throw load_exception("Size declared by DDS_HEADER is incorrect.");
      reader.read(this->flags);
      if (!(this->flags & header::flag::required_flags))
         throw load_exception("DDS_HEADER lacks some required flags.");
      reader.read(this->height);
      reader.read(this->width);
      reader.read(this->pitch);
      reader.read(this->depth);
      reader.read(this->mipmap_count);
      reader.read(this->reserved_a);
      reader.read(this->format);
      reader.read(this->capabilities);
      {
         auto cap = this->capabilities[1];
         if (cap & header::capabilities_1::is_cubemap) {
            if ((cap & header::capabilities_1::has_all_cubemap_faces) != header::capabilities_1::has_all_cubemap_faces) {
               throw load_exception("Partial cubemaps are not supported.");
            }
         }
      }
      reader.read(this->reserved_b);
      if (this->has_extended_header()) {
         reader.read(this->dx10_header);
      }
      //
      size_t data_start = header::serialized_size;
      if (this->has_extended_header())
         data_start += header_extension::serialized_size;
      //
      // Fix-ups:
      //
      if (this->mipmap_count) {
         //
         // GIMP-DDS incorrectly exports a mipmap count of 1 when exporting files without mipmaps.
         //
         size_t top_level_texture_size = 0;
         if (this->flags & header::flag::has_linear_size) {
            top_level_texture_size = this->linear_size;
         } else if (this->format.is_uncompressed()) {
            top_level_texture_size = this->width * this->height * this->format.rgb_bitcount;
         }
         if (top_level_texture_size) {
            if (this->size - data_start <= top_level_texture_size) {
               this->mipmap_count = 0;
            }
         }
      }
   }
   VkFormat header::to_vulkan_format() const {
      if (this->has_extended_header()) {
         auto i = this->dx10_header.format;
         if (i >= dxgi_formats.size())
            return VkFormat::VK_FORMAT_UNDEFINED;
         return dxgi_formats[i].vulkan.format;
      }
      if (this->format.has_four_cc()) {
         switch (this->format.four_cc) {
            case 'DXT1':
               return VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case 'DXT2':
               return VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;
            case 'DXT3':
               return VkFormat::VK_FORMAT_BC2_UNORM_BLOCK;
            case 'DXT4':
               return VkFormat::VK_FORMAT_BC3_UNORM_BLOCK;
            case 'DXT5':
               return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
         }
      }
      //
      // TODO: we can use the rest of the header to try and find a matching format
      //
      return VkFormat::VK_FORMAT_UNDEFINED;
   }
}