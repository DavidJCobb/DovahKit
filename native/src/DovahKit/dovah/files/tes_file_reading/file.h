#pragma once
#include <filesystem>
#include "basic_reader.h"
#include "threads.h"
#include "../common.h"
#include "../file_load_order.h"
#include "../file_read_error.h"

namespace dovah {
   class form_stub;
   namespace tes_file_writing {
      class file_writer;
   }

   namespace tes_file_reading {
      class localized_string_file;

      class file_reader : public basic_reader {
         //
         // Class used to load a single ESP/ESM/ESL file.
         //
         friend threads::basic;
         friend threads::interior_cell;
         friend threads::worldspace_sub_block;
         friend threads::worldspace_persistent_cell_children;
         public:
            using flag = tes_file_flag;
            struct detail_flag {
               detail_flag() = delete;
               enum type {
                  has_intv           = 0x0001,
                  has_incc           = 0x0002,
                  has_onam           = 0x0004,
                  is_hardcoded_dummy = 0x0008,
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
            static constexpr int threads_for_localization_load    = 1; // currently just here to be informative; there's no support for loading these on multiple threads yet
            //
            struct writer_interface {
               friend file_reader;
               protected:
                  const file_reader& wrapped;
                  writer_interface(file_reader& r) : wrapped(r) {}
               public:
                  inline const void* data_at(std::ptrdiff_t o) const noexcept {
                     return this->wrapped.file->data_at(o);
                  }
            };
            //
         public:
            file_reader(file_load_order&);
            ~file_reader();
            //
            bool load(const char* filepath);
            bool load_record_at(uint32_t pos); // for form_stub
            bool load_record_at(uint32_t pos, basic_reader* reader); // for form_stub (multi-threaded building of Use Info). the reader passed in must not be the "owner" of its mapped file. (this) will take ownership of (reader) by setting the latter's (owner).
            //
            bool open_mapped_file(const char* filepath = nullptr);
            void close(); // intended for use during the save process; not thread-safe; do not call during load
            //
         protected:
            bool _load_header();
            //
            std::filesystem::path path;
            std::string name;
            struct _readers {
               threads::basic complex;
               std::array<threads::basic,                               threads_for_simple_load>          simple;
               std::array<threads::interior_cell,                       threads_for_interior_cell_load>   interior_cell;
               std::array<threads::worldspace_sub_block,                threads_for_worldspace_load>      worldspace;
               std::array<threads::worldspace_persistent_cell_children, threads_for_worldspace_cell_load> world_cell;
               threads::localized_strings localized_strings;
               //
               _readers(file_reader&);
               void start();
               void wait_for();
               //
               float assess_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet
            } readers;
            //
            bool aborted = false;
            //
            void _insert_form(uint32_t formID, form_stub* stub);
            //
         public:
            file_load_order& load_order;
            file_read_error  error; // if the load process was aborted, what error, if any, did we encounter?
            uint32_t      flags   = 0;
            detail_flag_t details = 0;
            uint16_t      header_record_version = 0;
            float    fileVersion = 0.94F;
            uint32_t recordCount = 0;
            uint32_t nextFormID  = 0x00000800;
            std::string authorName;
            std::string description;
            std::vector<master_entry> masters;
            // TODO: ONAM, a list of overridden records within temporary CELLs, of the following types: ACHR, LAND, NAVM, REFR, PGRE, PHZD, PMIS, PARW, PBAR, PBEA, PCON, PFLA
            // TODO: DELE
            uint32_t subINTV;
            uint32_t subINCC;
            // TODO: SCRN
            struct {
               localized_string_file* common    = nullptr;
               localized_string_file* journal   = nullptr;
               localized_string_file* subtitles = nullptr;
            } localization_files;
            //
            inline const std::string& get_filename() const noexcept { return this->name; }
            inline const std::filesystem::path& get_path() const noexcept { return this->path; }
            void abort() noexcept;
            float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet

            writer_interface get_writer_interface(tes_file_writing::file_writer&) { return writer_interface(*this); }
            void set_path(const std::filesystem::path&); // also sets the filename. should only be used when saving an implicit active file
      };
   }
}