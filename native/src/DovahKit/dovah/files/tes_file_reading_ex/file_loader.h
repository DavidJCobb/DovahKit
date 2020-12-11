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

   class file_loader : basic_reader {
      using interface_t   = load_order_interfaces::file_load;
      using flag          = tes_file_flag;
      using detail_flag   = tes_file_header::detail_flag;
      using detail_flag_t = tes_file_header::detail_flag_t;
      public:
         file_loader(interface_t&);
         ~file_loader();
         //
         struct master_entry {
            std::string master; // MAST
            uint64_t    data;   // DATA
         };
         //
         static constexpr int threads_for_simple_load          = 4;
         static constexpr int threads_for_interior_cell_load   = 4;
         static constexpr int threads_for_worldspace_load      = 6;
         static constexpr int threads_for_worldspace_cell_load = 2;
         static constexpr int threads_for_localization_load    = 1; // currently just here to be informative; there's no support for loading these on multiple threads yet
         //
         tes_file_header header;
         localized_string_store* localization_data = nullptr; // owned
         //
         void  abort() noexcept;
         float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
         bool  load_record_at(uint32_t pos);
         std::string get_filename() const noexcept;
         //
         inline bool is_light() const noexcept { return this->header.is_light(); }
         //
         object_type next_record_or_group(); // only called during the initial file read
         bool        next_subrecord(); // called after the initial file read, when loading a form_stub's full content
         //
      protected:
         std::filesystem::path path;
         interface_t       load_interface;
         cobb::mapped_file file;
         //
         struct _readers {
            static_assert(false, "finish me");
            /*// commented out until they're defined, so as not to confuse intellisense
            threads::basic complex;
            std::array<threads::basic,                               threads_for_simple_load>          simple;
            std::array<threads::interior_cell,                       threads_for_interior_cell_load>   interior_cell;
            std::array<threads::worldspace_sub_block,                threads_for_worldspace_load>      worldspace;
            std::array<threads::worldspace_persistent_cell_children, threads_for_worldspace_cell_load> world_cell;
            threads::game_setting game_setting;
            //*/
            //
            _readers(file_loader&);
            void start();
            void wait_for();
            //
            float assess_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
         } readers;
         //
         bool aborted = false;
         //
         bool _load_header();
         //
      public:
         inline const cobb::mapped_file& get_raw_mapped_file() const noexcept { return this->file; };
         void log_load_warning(const file_part_loader& from, detailed_notice&);
         void log_load_error(const file_part_loader& from, detailed_notice&);
   };
}