#pragma once
#include <exception>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#include "../../../helpers/endian.h"
#include "../../../helpers/files.h"
#include "../../bs_hash.h"

#include "./archive_header.h"
#include "./packed_file_info.h"
#include "./packed_folder_info.h"

namespace dovah {
   class bsa_archived_file;
   class bsa_load_order;

   struct bsa_load_exception : public std::runtime_error {
      bsa_load_exception(const char* w) : runtime_error(w) {}

      std::filesystem::path bsa_path;
      uint64_t stream_position = 0;
   };
   struct bsa_unexpected_eof_exception : public bsa_load_exception {
      bsa_unexpected_eof_exception() : bsa_load_exception("unexpected EOF") {}
   };
   struct bsa_winapi_load_exception : public bsa_load_exception {
      bsa_winapi_load_exception() : bsa_load_exception("BSA WinAPI load error") {}

      uint32_t code = 0;
   };

   class bsa_archive {
      public:
         using file_entry   = bsa::packed_file_info;
         using folder_entry = bsa::packed_folder_info;

         static constexpr const char path_separator = '\\';

         static void normalize_path_or_path_component(std::string&);
         static constexpr void split_path_and_filename(const std::string& full_path, std::string& out_folder, std::string& out_file);
         
      protected:
         std::filesystem::path path;
         bsa::archive_header   header;
         cobb::mapped_file     mapping;
         std::vector<folder_entry> folders; // must be sorted by folder hash
         //
         // General loading-state fields:
         //
         uint64_t stream_position       = 0;
         bool     needs_endianness_flip = false;
         bool     loading_failed        = false;
         //
         // Fields for retrieving filenames from the filename blob:
         //
         uint32_t current_file_index    = 0;
         uint64_t filename_blob_offset  = 0;
         uint64_t last_filename_offset  = 0;

         template<typename T> requires (std::is_base_of_v<bsa_load_exception, T>) void _throw_load_exception() {
            this->loading_failed = true;
            auto e = T();
            e.bsa_path        = this->path;
            e.stream_position = this->stream_position;
            throw e;
         }
         template<typename T> requires (std::is_base_of_v<bsa_load_exception, T>) void _throw_load_exception(const char* w) {
            this->loading_failed = true;
            T e(w);
            e.bsa_path        = this->path;
            e.stream_position = this->stream_position;
            throw e;
         }

         void _unchecked_read(void* target, size_t size) noexcept;
         template<typename T> void _unchecked_read(T& out) noexcept {
            this->_unchecked_read(&out, sizeof(T));
            if (this->needs_endianness_flip)
               out = cobb::byteswap(out);
         }
         
         void _read(void* target, size_t size);
         template<typename T> void _read(T& out) {
            this->_read(&out, sizeof(T));
            if (this->needs_endianness_flip)
               out = cobb::byteswap(out);
         }
         void _read(std::string&);
         void _read(folder_entry&);
         void _read(file_entry&);

         void _read_non_null_terminated_string(std::string&, size_t length);

         // Intended for use post-load only.
         void _read_at(void* target, size_t size, uint64_t offset) const;
         template<typename T> void _read_at(T& out, uint64_t offset) const {
            this->_read_at(&out, sizeof(T), offset);
            if (this->needs_endianness_flip)
               out = cobb::byteswap(out);
         }
         
         constexpr bool is_eof() const noexcept {
            return this->stream_position >= this->mapping.size();
         }
         constexpr bool is_in_bounds(uint32_t bytes) const noexcept {
            return ((uint64_t)this->stream_position + bytes) < this->mapping.size();
         }
         
      public:
         void set_path(const std::filesystem::path&); // only works if a file is not open
         
         void open();
         void open(const std::filesystem::path&);

         // ---

         constexpr bool did_loading_fail() const noexcept { return this->loading_failed; }

         constexpr size_t folder_count() const noexcept { return this->folders.size(); }
         constexpr const decltype(folders)& folder_list() const noexcept { return this->folders; }
         
         constexpr const std::filesystem::path& get_path() const noexcept { return this->path; }
         bool is_open() const noexcept;

         // ---

         constexpr bool retains_pathnames() const;
         constexpr bool retains_filenames() const;

         constexpr bool packed_file_is_compressed(const file_entry&) const noexcept;

         constexpr bool for_each_folder(std::function<bool(const folder_entry&)> functor) const;
         constexpr bool for_each_file_in_folder(const folder_entry&, std::function<bool(const folder_entry&, const file_entry&)> functor) const;

         constexpr const bsa::packed_folder_info* lookup_folder_info(const bs_hash& folder_hash) const;
         constexpr const bsa::packed_folder_info* lookup_folder_info(const bs_hash& folder_hash, const std::string& folder_name) const;

         constexpr const bsa::packed_file_info* lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash) const;
         constexpr const bsa::packed_file_info* lookup_file_info(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& filename) const;

         bsa_archived_file* lookup_file(const bs_hash& folder, const bs_hash& file) const;
         bsa_archived_file* lookup_file(const std::string& path_and_name) const;

         bsa_archived_file* read_contents_of(const bsa::packed_file_info&) const;
   };
}

#include "bsa_archive.inl"