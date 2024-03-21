#pragma once
#include "./precached_nif_info.h"
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
      if constexpr (std::endian::native != std::endian::little) {
         this->extension = std::byteswap(this->extension);
      }
   }

   constexpr precached_nif_info::file_id::file_id(std::string_view full_path) {
      char extension[4] = { 0, 0, 0, 0 };

      size_t i = full_path.find_last_of(".\\/");
      if (i != std::string::npos) {
         if (full_path[i] == '.') {
            auto view = full_path.substr(i);
            for (size_t j = 0; j < 4 && j < view.size(); ++j)
               extension[j] = view[j];
         }
      }

      std::string_view filename(full_path);
      std::string_view folder;
      i = full_path.find_last_of("\\/");
      if (i != std::string::npos) {
         folder = filename.substr(0, i);
         filename = filename.substr(i + 1);
      }

      *this = file_id(folder, filename, extension);
   }
}
