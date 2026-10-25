#pragma once
#include "archive.h"
#include "lu/ascii/equals_i.h"

namespace dovah::bsa {
   template<typename Functor>
   constexpr bool archive::for_each_folder(Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&> {
      for (auto& folder : this->folders)
         if (functor(folder))
            return true;
      return false;
   }

   template<typename Functor>
   constexpr bool archive::for_each_file_in_folder(const folder_entry& folder, Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&, const file_entry&> {
      for (auto& file : folder.files)
         if (functor(folder, file))
            return true;
      return false;
   }

   constexpr const bsa::packed_folder_info* archive::lookup_folder_info(const bs_hash& folder_hash) const {
      for (auto& folder : this->folders) {
         if (folder.hash > folder_hash)
            break;
         if (folder.hash == folder_hash)
            return &folder;
      }
      return nullptr;
   }
   constexpr const packed_folder_info* archive::lookup_folder_info(const bs_hash& folder_hash, const std::string& folder_name) const {
      for (auto& folder : this->folders) {
         if (folder.hash > folder_hash)
            break;
         if (folder.hash == folder_hash)
            if (lu::ascii::equals_i(folder.name, folder_name))
               return &folder;
      }
      return nullptr;
   }

   constexpr const packed_file_info* archive::lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash) const {
      auto* folder = this->lookup_folder_info(folder_hash);
      if (!folder)
         return nullptr;
      return folder->lookup_file_info(file_hash);
   }
   constexpr const packed_file_info* archive::lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& filename) const {
      auto* folder = this->lookup_folder_info(folder_hash, folder_name);
      if (!folder)
         return nullptr;
      return folder->lookup_file_info(file_hash, filename);
   }

   constexpr bool archive::packed_file_is_compressed(const file_entry& file) const noexcept {
      bool result = file.non_default_compression();
      if (this->header.flags.has_all_of<archive_header_flag::compressed_by_default>())
         result = !result;
      return result;
   }

   constexpr bool archive::retains_pathnames() const {
      return this->header.flags.has_all_of<archive_header_flag::include_directory_names>();
   }
   constexpr bool archive::retains_filenames() const {
      return this->header.flags.has_any_of<archive_header_flag::embed_filenames, archive_header_flag::include_filenames>();
   }
}