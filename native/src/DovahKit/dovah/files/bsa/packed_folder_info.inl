#pragma once
#include "./packed_folder_info.h"
#include "helpers/string/strieq_ascii.h"

namespace dovah::bsa {
   constexpr const packed_file_info* packed_folder_info::lookup_file_info(const bs_hash& file_hash) const noexcept {
      for (auto& file : this->files) {
         if (file.hash > file_hash)
            return nullptr;
         if (file.hash == file_hash)
            return &file;
      }
      return nullptr;
   }
   constexpr const packed_file_info* packed_folder_info::lookup_file_info(const bs_hash& file_hash, const std::string& filename) const noexcept {
      for (auto& file : this->files) {
         if (file.hash > file_hash)
            return nullptr;
         if (file.hash == file_hash) {
            if (!filename.empty() && !file.name.empty()) {
               if (!cobb::strieq_ascii(filename, file.name))
                  continue;
            }
            return &file;
         }
      }
      return nullptr;
   }


   constexpr const packed_file_info* packed_folder_info::find_file(const bs_hash& file_hash, const std::string& file_name) const noexcept {
      for (auto& file : this->files) {
         if (file.hash > file_hash)
            return nullptr;
         if (file.hash == file_hash) {
            if (!file_name.empty() && !file.name.empty()) {
               if (!cobb::strieq_ascii(file_name, file.name))
                  continue;
            }
            return &file;
         }
      }
      return nullptr;
   }
}

