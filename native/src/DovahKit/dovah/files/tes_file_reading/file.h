#pragma once
#include "basic_reader.h"
#include "threads.h"
#include "../file_load_order.h"

namespace dovah {
   class  form_stub;

   namespace tes_file_reading {
      class file_reader : public basic_reader {
         //
         // Class used to load a single ESP/ESM/ESL file.
         //
         friend threads::basic;
         friend threads::interior_cell;
         friend threads::worldspace_sub_block;
         friend threads::worldspace_persistent_cell_children;
         public:
            struct flag {
               flag() = delete;
               enum type {
                  master                 = 0x0001,
                  localized_string_table = 0x0080,
                  light                  = 0x0200, // SSE-only
               };
            };
            struct detail_flag {
               detail_flag() = delete;
               enum type {
                  has_intv = 0x0001,
                  has_incc = 0x0002,
                  has_onam = 0x0004,
               };
            };
            using detail_flag_t = std::underlying_type_t<detail_flag::type>;
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
            //
         public:
            file_reader(file_load_order&);
            ~file_reader();
            //
            bool load(const char* filepath);
            bool load_record_at(uint32_t pos); // for FormStub
            bool load_record_at(uint32_t pos, basic_reader* reader); // for FormStub (multi-threaded building of Use Info); the reader passed in must not be the "owner" of its mapped file
            //
         protected:
            bool _load_header();
            //
            std::string path;
            std::string name;
            struct _readers {
               threads::basic complex;
               std::array<threads::basic,                               threads_for_simple_load>          simple;
               std::array<threads::interior_cell,                       threads_for_interior_cell_load>   interior_cell;
               std::array<threads::worldspace_sub_block,                threads_for_worldspace_load>      worldspace;
               std::array<threads::worldspace_persistent_cell_children, threads_for_worldspace_cell_load> world_cell;
               //
               _readers(file_reader&);
               void start();
               void wait_for();
            } readers;
            //
            bool aborted = false;
            //
            void _insert_form(uint32_t formID, form_stub* stub);
            //
         public:
            file_load_order& load_order;
            uint32_t     flags = 0;
            detail_flag_t details = 0;
            float    fileVersion = 0.94F;
            uint32_t recordCount = 0;
            uint32_t nextFormID;
            char     authorName[512];
            char     description[512];
            std::vector<master_entry> masters;
            // TODO: ONAM
            uint32_t subINTV;
            uint32_t subINCC;
            //
            inline const std::string& get_filename() const noexcept { return this->name; }
            void abort() noexcept;
      };
   }
}