#pragma once
#include <filesystem>
#include <string>
#include <map>
#include <vector>
#include "../../../helpers/files.h"
#include "../../bs_hash.h"

namespace dovah {
   class bsa_archived_file;
   class bsa_load_order;

   struct bsa_header {
      struct flag {
         flag() = delete;
         enum : uint32_t {
            include_directory_names = 0x00000001,
            include_filenames       = 0x00000002,
            compressed_by_default   = 0x00000004,
            retain_directory_names  = 0x00000008, // guides run-time behavior; no effect on file structure
            retain_filenames        = 0x00000010, // guides run-time behavior; no effect on file structure
            retain_filename_offsets = 0x00000020, // guides run-time behavior; no effect on file structure
            big_endian              = 0x00000040, // for Xbox 360
            retain_strings          = 0x00000080, // guides run-time behavior; no effect on file structure
            embed_filenames         = 0x00000100,
            use_xmem_codec          = 0x00000200, // indicates use of the Xbox 360-exclusive XMem compression algorithm
            //
            all_mandatory_flags = include_directory_names | include_filenames
         };
      };
      struct filetype {
         filetype() = delete;
         enum : uint32_t {
            meshes        = 0x00000001,
            textures      = 0x00000002,
            menus         = 0x00000004,
            sounds        = 0x00000008,
            voices        = 0x00000010,
            shaders       = 0x00000020,
            trees         = 0x00000040,
            fonts         = 0x00000080,
            miscellaneous = 0x00000100,
         };
      };
      struct version {
         version() = delete;
         enum : uint32_t {
            oblivion          = 0x67,
            fallout_3         = 0x68,
            fallout_new_vegas = 0x68,
            skyrim_classic    = 0x68,
            skyrim_special    = 0x69,
         };
      };
      //
      uint32_t sentinel      = _byteswap_ulong('BSA\0');
      uint32_t version       = version::skyrim_classic;
      uint32_t folder_offset = 0x24;
      uint32_t flags         = flag::all_mandatory_flags;
      uint32_t folder_count  = 0;
      uint32_t file_count    = 0;
      uint32_t total_folder_name_length = 0; // total length of all folder names, including null-terminators but not including length prefix bytes
      uint32_t total_filename_length    = 0; // total length of all filenames, including null-terminators
      uint32_t filetypes = 0;
      //
      static constexpr int size_of_header = 0x24;
      
      //
      // The typical file layout is:
      //  - Header
      //  - Folder metadata
      //     - Folder [array]
      //        - Name hash
      //        - Count
      //        - Offset(s)
      //  - Folder data
      //     - Folder name (if "include_directory_names" flag is set)
      //     - File metadata [array]
      //        - Name hash
      //        - Size and flags
      //        - Offset
      //  - Filename blob (if "include_filenames" flag is set)
      //     - Blob of filenames, matched to files by index (all folders and files are sorted by hash)
      //  - Files
      //     - Full path and filename, if "embed_filenames" flag is set
      //     - Raw file data
      //

      inline uint32_t folder_metadata_total_size() const noexcept {
         int folder_size = 16;
         if (this->version >= version::skyrim_special)
            folder_size += 8;
         return folder_size * this->folder_count;
      }
      inline uint32_t folder_data_total_size() const noexcept {
         int file_metadata_size = 16;
         return (file_metadata_size * this->file_count) + this->total_folder_name_length + this->folder_count; // add the folder count to account for folder names' length prefix bytes
      }

      inline uint32_t expected_folder_data_position() const noexcept {
         return this->folder_offset + this->folder_metadata_total_size();
      }
      inline uint32_t expected_filename_blob_position() const noexcept {
         return this->expected_folder_data_position() + this->folder_data_total_size();
      }
   };

   class bsa_archive {
      public:
         enum class read_error_code {
            none,
            bad_header_sentinel, // the header sentinel was not 'B' 'S' 'A' '\0'
         };
         //
      protected:
         struct file_entry {
            bs_hash     hash;
            std::string name; // present only if the BSA uses the (include_filenames) flag
            uint32_t    size_and_flags = 0;
            uint32_t    offset         = 0; // offset of: the embedded filename, if any, with a one-byte length prefix and no null terminator; the uncompressed size (if the file is compressed); and then the file data
            //
            inline uint32_t size() const noexcept { return this->size_and_flags & ~0xC0000000; }
            inline bool invalidated() const noexcept { return (this->size_and_flags & 0x80000000) != 0; } // this flag should only be set at run-time
            inline bool non_default_compression() const noexcept { return (this->size_and_flags & 0x40000000) != 0; }
         };
         struct folder_entry {
            bs_hash     hash;
            std::string name; // present only if the BSA uses the (include_directory_names) flag
            uint64_t    offset  = 0; // is uint32_t in Classic and a uint64_t in Special
            uint32_t    padding = 0; // Special adds a uint64_t to the end of the struct, so the legacy offset field becomes padding
            std::vector<file_entry> files; // must be sorted by file hash
            //
            const file_entry* find_file(const bs_hash& file_hash, const std::string& file_name) const noexcept; // if string args are non-empty and the archive contains strings, then args are used to verify hash correctness
         };
         //
         bsa_header        header;
         cobb::mapped_file mapping;
         std::vector<folder_entry> folders; // must be sorted by folder hash
         //
         // General loading-state fields:
         //
         read_error_code read_error = read_error_code::none;
         uint64_t stream_position       = 0;
         bool     needs_endianness_flip = false;
         //
         // Fields for retrieving filenames from the filename blob:
         //
         uint32_t current_file_index    = 0;
         uint64_t filename_blob_offset  = 0;
         uint64_t last_filename_offset  = 0;
         //
         void _read(void* target, size_t size);
         template<typename T> void _read(T& out) {
            this->_read(&out, sizeof(T));
            if (this->needs_endianness_flip)
               out = cobb::byteswap(out);
         }
         void _read(std::string&);
         void _read(folder_entry&);
         void _read(file_entry&);
         //
         void _read_at(void* target, size_t size, uint64_t offset);
         template<typename T> void _read_at(T& out, uint64_t offset) {
            this->_read(&out, sizeof(T), offset);
            if (this->needs_endianness_flip)
               out = cobb::byteswap(out);
         }
         //
         void _read_non_null_terminated_string(std::string&, size_t length);
         //
         const file_entry* find_file(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& file_name) const noexcept; // if string args are non-empty and the archive contains strings, then args are used to verify hash correctness
         bsa_archived_file* retrieve_entry(const file_entry&);
         //
      public:
         void open(const std::filesystem::path&);
         //
         bsa_archived_file* lookup_file(const bs_hash& folder, const bs_hash& file);
         bsa_archived_file* lookup_file(const std::string& path_and_name);
   };
}