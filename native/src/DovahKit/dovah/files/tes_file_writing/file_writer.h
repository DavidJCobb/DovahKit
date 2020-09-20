#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "elements.h"

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
            enum class compression_policy {
               never,     // never compress records
               threshold, // compress records that are larger than a certain size
               bethesda,  // compress NAVM, NPC_, any CELL that has TVDT, and any LAND in a compressed CELL, all regardless of the records' sizes
            };
            //
            struct form_stub_write_info {
               //
               // This data should be transplanted into a form_stub after the full file write is complete.
               //
               uint32_t offset = 0;
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
            std::unordered_map<bare_form_id_t, form_stub_write_info> stub_writes;
            struct {
               struct {
                  file_offset_t record_count   = 0;
                  file_offset_t next_object_id = 0;
               } header;
               struct {
                  uint32_t      value = 0;
                  file_offset_t offset = 0;
               } record_and_group_count;
            } fixup_data;
            //
            record& _open_next_record(uint32_t signature, bare_form_id_t);
            void _write_header();
            bool _write_form(form_stub*);
            void _write_record();
            void _write_child_forms_for_cell(form_stub*);
            void _write_child_forms_for_topic(form_stub*);
            void _write_child_forms_for_worldspace(form_stub*);
            void _write_interior_cells();
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
         public:
            file_writer(file_load_order&, file_reader&);
            ~file_writer();
            //
            union {
               struct {
                  uint8_t vc_day;
                  uint8_t vc_month;
                  uint8_t vc_last_editor;
                  uint8_t vc_current_editor;
               };
               uint32_t version_control = 0;
            };
            uint16_t version_control_2 = 0;
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

            uint32_t get_stream_position() const noexcept;
            void set_stream_position(file_offset_t) noexcept;

            void open(std::filesystem::path);
            void write();
      };
   }
}