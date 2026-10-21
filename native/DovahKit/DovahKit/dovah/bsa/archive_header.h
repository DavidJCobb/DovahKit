#pragma once
#include <cstdint>
#include "lu/integral_character_code.h"
#include "./archive_filetype_flags.h"
#include "./archive_header_flags.h"
#include "./archive_version.h"

namespace dovah::bsa {
   struct archive_header {
      using flag       = archive_header_flag;
      using flags_type = archive_header_flags;

      using filetype_flag       = archive_filetype_flag;
      using filetype_flags_type = archive_filetype_flags;
      
   public:
      lu::four_cc         sentinel      = 'BSA\0';
      archive_version     version       = archive_version::skyrim_classic;
      uint32_t            folder_offset = 0x24;
      flags_type          flags         = all_mandatory_archive_header_flags;
      uint32_t            folder_count  = 0;
      uint32_t            file_count    = 0;
      uint32_t            total_folder_name_length = 0; // total length of all folder names, including null-terminators but not including length prefix bytes
      uint32_t            total_filename_length    = 0; // total length of all filenames, including null-terminators
      filetype_flags_type filetypes;
      
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
         if (this->version >= archive_version::skyrim_special)
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
