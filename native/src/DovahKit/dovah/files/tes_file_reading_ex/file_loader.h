#pragma once
#include <filesystem>
#include "../../../helpers/files.h"
#include "../common.h"
#include "basic_reader.h"
#include "threads.h"
#include "../../localization/localized_string_store.h"
#include "../file_header.h"
#include "../file_load_order.h"

namespace dovah::tes_file_reading {
   class file_part_loader;
   class file_threaded_part_loader_base;

   class file_loader : basic_reader {
      using interface_t   = load_order_interfaces::file_load;
      using flag          = tes_file_flag;
      using detail_flag   = tes_file_header::detail_flag;
      using detail_flag_t = tes_file_header::detail_flag_t;
      public:
         file_loader(interface_t&); // TODO: threaded loaders should be created in the constructor, and so should never be nullptr during this object's lifetime
         ~file_loader();
         //
         struct master_entry {
            std::string master; // MAST
            uint64_t    data;   // DATA
         };
         //
         static constexpr int threads_for_simple_load          = 4;
         static constexpr int threads_for_dialogue_load        = 1;
         static constexpr int threads_for_interior_cell_load   = 4;
         static constexpr int threads_for_worldspace_load      = 6;
         static constexpr int threads_for_worldspace_cell_load = 2;
         static constexpr int threads_for_game_settings        = 1;
         static constexpr int total_threads = (
            threads_for_simple_load
          + threads_for_dialogue_load
          + threads_for_interior_cell_load
          + threads_for_worldspace_load
          + threads_for_worldspace_cell_load
          + threads_for_game_settings
         );
         //
         tes_file_header header;
         localized_string_store* localization_data = nullptr; // pointer is owned by the (file_loader) once received
         //
         void  abort() noexcept;
         float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
         bool  fetch_record_header(uint32_t pos, tes_file_record_header&, uint32_t& record_decompressed_size);
         bool  load_record_at(uint32_t pos);
         std::string get_filename() const noexcept;
         //
         inline bool is_light() const noexcept { return this->header.is_light(); }
         //
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
         bool load(const std::filesystem::path&); // path is optional; if empty, reuses prior path (if any). calling this while a load is already in progress is undefined behavior
         void close(); // intended for use during the save process, with the file then being reopened by the caller upon a successful save
         //
      protected:
         std::filesystem::path path;
         interface_t       load_interface;
         cobb::mapped_file file;
         std::vector<file_threaded_part_loader_base*> threads; // array elements should never be nullptr after the instance is constructed.
         //
         bool aborted = false; // TODO: make atomic?
         //
         bool _load_header();
         bool _start_threads();
         bool _wait_for_threads();
         //
      public:
         inline const cobb::mapped_file& get_raw_mapped_file() const noexcept { return this->file; };
         void log_load_warning(const file_part_loader& from, detailed_notice&);
         void log_load_error(const file_part_loader& from, detailed_notice&);
   };
}