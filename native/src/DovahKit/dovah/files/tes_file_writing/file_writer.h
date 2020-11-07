#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "config.h"
#include "elements.h"
#include "../file_write_error.h"

namespace dovah {
   class file_load_order;
   namespace tes_file_reading {
      class file_reader;
   }

   namespace tes_file_writing {
      class file_writer {
         friend group;
         friend record;
         friend subrecord;
         public:
            using file_reader   = tes_file_reading::file_reader;
            using file_offset_t = uint32_t;
            using stream_t      = std::basic_ofstream<uint8_t>;
            static constexpr int max_group_depth = 7;
            //
            struct form_stub_write_info {
               //
               // This data should be transplanted into a form_stub after the full file write is complete.
               //
               form_stub* stub   = nullptr;
               uint32_t   offset = 0;
            };
            //
         protected:
            mutable stream_t stream; // ofstream::tellp and friends aren't const despite not modifying the stream state
            file_load_order& owner;
            file_reader&     source;
            //
            std::array<group, max_group_depth> _groups;
            record    _record;
            subrecord _subrecord;
            struct {
               bool containing_cell_is_compressed = false;
            } compress_state;
            //
            record& _open_next_record(uint32_t signature, bare_form_id_t);
            bool _should_compress_current_record(form_stub* stub = nullptr) const noexcept;
            void _write_header();
            bool _write_form(form_stub*);
            void _write_record(form_stub* stub = nullptr); // pass the stub when writing forms, for error reporting purposes
            void _write_child_forms_for_cell(form_stub*);
            void _write_child_forms_for_topic(form_stub*);
            void _write_child_forms_for_worldspace(form_stub*);
            void _write_interior_cells();
            void _write_game_settings();
            //
            void _write_impl(const void* source, uint32_t size);
            void _write_impl(const tes_file_group_header&);
            void _write_impl(const tes_file_record_header&);
            template<typename T> inline void _write(const T& v) {
               this->_write_impl(&v, sizeof(T));
            }
            template<> inline void _write(const tes_file_group_header& v) {
               this->_write_impl(v);
            }
            template<> inline void _write(const tes_file_record_header& v) {
               this->_write_impl(v);
            }
            //
            bool _can_serialize_form(const form_stub*) const noexcept;
            //
         public:
            file_writer(file_load_order&, file_reader&, const write_config& cfg);
            ~file_writer();
            //
            #pragma region config
            write_config config;
            bool use_string_table; // constructor defaults this to whatever the source file did
            #pragma endregion
            //
            file_write_error error;
            struct {
               struct {
                  file_offset_t record_count   = 0;
                  file_offset_t next_object_id = 0;
               } header;
               struct {
                  uint32_t      value = 0;
                  file_offset_t offset = 0;
               } record_and_group_count;
               std::unordered_map<bare_form_id_t, form_stub_write_info> form_stubs;
            } fixup_data;
            //
            inline group& get_current_group() {
               for (signed int i = this->_groups.size() - 1; i >= 0; i--) {
                  auto& group = this->_groups[i];
                  if (group)
                     return group;
               }
               return this->_groups[0];
            }
            inline record& get_current_record() { return this->_record; }
            inline subrecord& get_current_subrecord() { return this->_subrecord; }

            group& open_group(tes_file_group_type, uint32_t label, uint32_t unknown = 0);
            void close_current_group();

            uint32_t get_stream_position() const noexcept; // position in the file. note that (sub)record writes don't advance this until the record is closed.
            void set_stream_position(file_offset_t) noexcept;
            uint32_t get_output_position() const noexcept; // stream position + record position if open + subrecord position if open. WARNING: this can't account for large subrecords that end up using 'XXXX'

            void open(std::filesystem::path);
            bool write();
            void update_source_file_header();
            void close();
      };
   }
}