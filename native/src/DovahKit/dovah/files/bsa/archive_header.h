#pragma once
#include <cstdint>

namespace dovah::bsa {
   struct archive_header {
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
      
   public:
      uint32_t sentinel      = _byteswap_ulong('BSA\0');
      uint32_t version       = version::skyrim_classic;
      uint32_t folder_offset = 0x24;
      uint32_t flags         = flag::all_mandatory_flags;
      uint32_t folder_count  = 0;
      uint32_t file_count    = 0;
      uint32_t total_folder_name_length = 0; // total length of all folder names, including null-terminators but not including length prefix bytes
      uint32_t total_filename_length    = 0; // total length of all filenames, including null-terminators
      uint32_t filetypes = 0;
      
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

      constexpr uint32_t folder_metadata_total_size() const noexcept {
         int folder_size = 16;
         if (this->version >= version::skyrim_special)
            folder_size += 8;
         return folder_size * this->folder_count;
      }
      constexpr uint32_t folder_data_total_size() const noexcept {
         int file_metadata_size = 16;
         return (file_metadata_size * this->file_count) + this->total_folder_name_length + this->folder_count; // add the folder count to account for folder names' length prefix bytes
      }

      constexpr uint32_t expected_folder_data_position() const noexcept {
         return this->folder_offset + this->folder_metadata_total_size();
      }
      constexpr uint32_t expected_filename_blob_position() const noexcept {
         return this->expected_folder_data_position() + this->folder_data_total_size();
      }
   };
}
