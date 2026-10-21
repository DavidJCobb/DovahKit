#pragma once
#include "./packed_folder_info.h"
#include "lu/ascii/equals_i.h"

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
               if (!lu::ascii::equals_i(filename, file.name))
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
               if (!lu::ascii::equals_i(file_name, file.name))
                  continue;
            }
            return &file;
         }
      }
      return nullptr;
   }
}

