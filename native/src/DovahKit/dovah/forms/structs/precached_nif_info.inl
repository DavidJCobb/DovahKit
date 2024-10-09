#pragma once
#include "./precached_nif_info.h"
#include <string>
#include "helpers/hashing/crc32.h"

namespace dovah::loaded_forms {
   template<size_t ExtensionLength> requires (ExtensionLength <= 4)
   constexpr precached_nif_info::file_id::file_id(std::string_view folder, std::string_view filename, const char(&extension)[ExtensionLength]) {
      constexpr const cobb::hashing::crc32_params params = {
         .initial_value = 0,
         .invert_bits   = false,
         .polynomial    = 0xEDB88320,
      };
      constexpr const auto functor = [](char c) constexpr noexcept -> char {
         if (c >= 'A' && c <= 'Z')
            return c + 0x20;
         if (c == '/')
            return '\\';
         return c;
      };

      this->file_hash   = cobb::hashing::crc32<params, functor>(filename);
      this->folder_hash = cobb::hashing::crc32<params, functor>(folder);

      this->set_extension(extension);
   }

   constexpr precached_nif_info::file_id::file_id(std::string_view full_path) {
      char extension[4] = { 0, 0, 0, 0 };

      size_t i = full_path.find_last_of(".\\/");
      if (i != std::string::npos && i + 1 < full_path.size()) {
         if (full_path[i] == '.') {
            auto view = full_path.substr(i + 1);
            for (size_t j = 0; j < 4 && j < view.size(); ++j)
               extension[j] = view[j];
         }
      }

      std::string_view filename(full_path);
      std::string_view folder;
      size_t j = full_path.find_last_of("\\/");
      if (j != std::string::npos) {
         folder   = filename.substr(0, j);
         filename = filename.substr(j + 1);
      }
      if (i != std::string::npos) { // filename does not include the extension
         filename = filename.substr(0, i - j - 1);
      }

      *this = file_id(folder, filename, extension);
   }

   template<size_t ExtensionLength> requires (ExtensionLength <= 4)
   constexpr void precached_nif_info::file_id::set_extension(const char(&extension)[ExtensionLength]) {
      this->extension = 0;

      if constexpr (ExtensionLength > 0) {
         this->extension |= extension[0];
         if constexpr (ExtensionLength > 1) {
            this->extension |= extension[1] << 0x08;
            if constexpr (ExtensionLength > 2) {
               this->extension |= extension[2] << 0x10;
               if constexpr (ExtensionLength > 3) {
                  this->extension |= extension[3] << 0x18;
               }
            }
         }
      }
      //
      // Given `extension == AA BB CC DD`, our value is now:
      // 
      //  - AA BB CC DD if the native endianness is little
      //  - DD CC BB AA if the native endianness is big
      //
      if constexpr (std::endian::native != std::endian::little) {
         this->extension = std::byteswap(this->extension);
      }
   }
   
   static_assert(
      []() constexpr -> bool {
         auto id = precached_nif_info::file_id("textures\\trap\\BearTrap.dds");
         return id.file_hash == 0x94310BFB;
      }(),
      "Unit test: file hash (BearTrap.dds)."
   );
   static_assert(
      []() constexpr -> bool {
         auto id = precached_nif_info::file_id("textures\\trap\\BearTrap_n.dds");
         return id.file_hash == 0xF7B3780B;
      }(),
      "Unit test: file hash (BearTrap_n.dds)."
   );
   static_assert(
      []() constexpr -> bool {
         uint32_t desired = 'dds\0';
         if constexpr (std::endian::native == std::endian::little)
            desired = std::byteswap(desired);

         auto id = precached_nif_info::file_id("textures\\trap\\BearTrap.dds");
         return id.extension == desired;
      }(),
      "Unit test: extension."
   );
   static_assert(
      []() constexpr -> bool {
         auto id = precached_nif_info::file_id("textures\\trap\\BearTrap.dds");
         return id.folder_hash == 0x13AE15E1;
      }(),
      "Unit test: folder hash."
   );
}
