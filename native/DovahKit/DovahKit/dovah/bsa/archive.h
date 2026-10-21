#pragma once
#include <bit>
#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>
#include "lu/files/mapped_file.h"
#include "dovah/types/bs_hash.h"

#include "./archive_header.h"
#include "./packed_file_info.h"
#include "./packed_folder_info.h"
namespace dovah {
   namespace bsa {
      class bytereader;
   }
   class asset_file_buffer;
}

namespace dovah::bsa {
   class archive {
      public:
         using file_entry   = bsa::packed_file_info;
         using folder_entry = bsa::packed_folder_info;

      protected:
         std::filesystem::path     path;
         archive_header            header;
         lu::files::mapped_file    mapping;
         std::vector<folder_entry> folders; // must be sorted by folder hash
         //
         // Fields for retrieving filenames from the filename blob:
         //
         uint32_t current_file_index   = 0; // for debugging
         size_t   filename_blob_offset = 0;
         size_t   last_filename_offset = 0;

         void _read(bytereader&, folder_entry&);
         void _read(bytereader&, file_entry&);
         
      public:
         void set_path(std::filesystem::path&&); // only works if a file is not open
         void set_path(const std::filesystem::path&); // only works if a file is not open
         
         // Throws `std::system_error` if the file fails to open.
         // Throws `dovah::exceptions::bsa_parse_error` if the file is ill-formed.
         void open();
         void open(const std::filesystem::path&);

         // ---

         constexpr size_t folder_count() const noexcept { return this->folders.size(); }
         constexpr const decltype(folders)& folder_list() const noexcept { return this->folders; }
         
         constexpr const std::filesystem::path& get_path() const noexcept { return this->path; }
         bool is_open() const noexcept;

         // ---

         constexpr bool retains_pathnames() const;
         constexpr bool retains_filenames() const;

         constexpr bool packed_file_is_compressed(const file_entry&) const noexcept;

         template<typename Functor>
         constexpr bool for_each_folder(Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&>;
         
         template<typename Functor>
         constexpr bool for_each_file_in_folder(const folder_entry&, Functor&& functor) const requires std::is_invocable_r_v<bool, Functor, const folder_entry&, const file_entry&>;

         constexpr const bsa::packed_folder_info* lookup_folder_info(const bs_hash& folder_hash) const;
         constexpr const bsa::packed_folder_info* lookup_folder_info(const bs_hash& folder_hash, const std::string& folder_name) const;

         constexpr const bsa::packed_file_info* lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash) const;
         constexpr const bsa::packed_file_info* lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& filename) const;

         // These throw `dovah::exceptions::bsa_extract_error` for some failures.
         // They return an empty object if the file doesn't exist.
         asset_file_buffer lookup_file(const bs_hash& folder, const bs_hash& file) const;
         asset_file_buffer lookup_file(const std::string& path_and_name) const;
         asset_file_buffer read_contents_of(const bsa::packed_file_info&) const;
   };
}

#include "./archive.inl"