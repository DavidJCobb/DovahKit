#pragma once
#include <filesystem>
#include "basic_reader.h"
#include "threads.h"
#include "../common.h"
#include "../file_header.h"
#include "../file_load_order.h"
#include "../file_read_error.h"

namespace dovah {
   class form_stub;
   class localized_string_store;
   namespace load_order_interfaces {
      class file_load;
   }
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
         friend threads::game_setting;
         public:
            using flag = tes_file_flag;
            using detail_flag   = tes_file_header::detail_flag;
            using detail_flag_t = tes_file_header::detail_flag_t;
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
            bool load(const char* filepath, load_order_interfaces::file_load& intfc);
            bool load_record_at(uint32_t pos); // for form_stub
            bool load_record_at(uint32_t pos, basic_reader* reader); // for form_stub (multi-threaded building of Use Info). the reader passed in must not be the "owner" of its mapped file. (this) will take ownership of (reader) by setting the latter's (owner).
            bool fetch_record_header(uint32_t pos, tes_file_record_header&, uint32_t& record_decompressed_size);
            //
            bool open_mapped_file(const char* filepath = nullptr, detailed_notice* out_error_if_any = nullptr);
            void close(); // intended for use during the save process; not thread-safe; do not call during load
            //
         protected:
            bool _load_header(load_order_interfaces::file_load& intfc);
            //
            std::filesystem::path path;
            std::string name;
            struct _readers {
               threads::basic complex;
               std::array<threads::basic,                               threads_for_simple_load>          simple;
               std::array<threads::interior_cell,                       threads_for_interior_cell_load>   interior_cell;
               std::array<threads::worldspace_sub_block,                threads_for_worldspace_load>      worldspace;
               std::array<threads::worldspace_persistent_cell_children, threads_for_worldspace_cell_load> world_cell;
               threads::game_setting game_setting;
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
            [[nodiscard]] bool _insert_form(uint32_t formID, form_stub* stub);
            //
         public:
            file_load_order& load_order;
            file_read_error  error; // if the load process was aborted, what error, if any, did we encounter?
            tes_file_header  header;
            localized_string_store* localization_data = nullptr; // owned
            //
            inline const std::string& get_filename() const noexcept { return this->name; }
            inline const std::filesystem::path& get_path() const noexcept { return this->path; }
            void abort() noexcept;
            float assess_load_progress() const noexcept; // returns NaN if any threaded reader hasn't set up its maximum yet

            writer_interface get_writer_interface(tes_file_writing::file_writer&) { return writer_interface(*this); }
            void set_path(const std::filesystem::path&); // also sets the filename. should only be used when saving an implicit active file

            inline bool is_light() const noexcept { return this->header.is_light(); }
      };
   }
}