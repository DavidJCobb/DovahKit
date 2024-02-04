#pragma once
#include "bsa_archive.h"

namespace dovah {
   /*static*/ constexpr void bsa_archive::split_path_and_filename(const std::string& full_path, std::string& out_folder, std::string& out_file) {
      size_t size = full_path.size();
      for (char c : full_path) {
         if (c == '\\' || c == '/') {
            if (out_file.empty()) // skip leading and redundant slashes
               continue;
            if (!out_folder.empty())
               out_folder += '\\';
            out_folder += out_file;
            out_file.clear();
         } else {
            if (c >= 'A' && c <= 'Z')
               c += 0x20;
            out_file += c;
         }
      }
   }

   template<typename Functor>
   constexpr bool bsa_archive::for_each_folder(Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&> {
      for (auto& folder : this->folders)
         if (functor(folder))
            return true;
      return false;
   }

   template<typename Functor>
   constexpr bool bsa_archive::for_each_file_in_folder(const folder_entry& folder, Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&, const file_entry&> {
      for (auto& file : folder.files)
         if (functor(folder, file))
            return true;
      return false;
   }

   constexpr const bsa::packed_folder_info* bsa_archive::lookup_folder_info(const bs_hash& folder_hash) const {
      for (auto& folder : this->folders) {
         if (folder.hash > folder_hash)
            break;
         if (folder.hash == folder_hash)
            return &folder;
      }
      return nullptr;
   }
   constexpr const bsa::packed_folder_info* bsa_archive::lookup_folder_info(const bs_hash& folder_hash, const std::string& folder_name) const {
      for (auto& folder : this->folders) {
         if (folder.hash > folder_hash)
            break;
         if (folder.hash == folder_hash)
            if (cobb::strieq_ascii(folder.name, folder_name))
               return &folder;
      }
      return nullptr;
   }

   constexpr const bsa::packed_file_info* bsa_archive::lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash) const {
      auto* folder = this->lookup_folder_info(folder_hash);
      if (!folder)
         return nullptr;
      return folder->lookup_file_info(file_hash);
   }
   constexpr const bsa::packed_file_info* bsa_archive::lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& filename) const {
      auto* folder = this->lookup_folder_info(folder_hash, folder_name);
      if (!folder)
         return nullptr;
      return folder->lookup_file_info(file_hash, filename);
   }

   constexpr bool bsa_archive::packed_file_is_compressed(const file_entry& file) const noexcept {
      bool result = file.non_default_compression();
      if (this->header.flags & bsa::archive_header::flag::compressed_by_default)
         result = !result;
      return result;
   }

   constexpr bool bsa_archive::retains_pathnames() const {
      return (this->header.flags & bsa::archive_header::flag::include_directory_names) != 0;
   }
   constexpr bool bsa_archive::retains_filenames() const {
      return (this->header.flags & (bsa::archive_header::flag::embed_filenames | bsa::archive_header::flag::include_filenames)) != 0;
   }
}