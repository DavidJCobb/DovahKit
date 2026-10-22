#pragma once
#include <array>
#include "lu/memory/basic_buffer.h"
#include "../../types/bare_form_id.h"
#include "../bytereader.h"
#include "../group_header.h"
#include "./record_view.h"
#include "./subrecord_view.h"
namespace dovah::tes {
   class file;
}

namespace dovah::tes {
   class file_view {
      public:
         file_view();

         static file_view open_view_to_group(const file&, size_t group_header_offset);
         static file_view open_view_to_record(const file&, size_t record_header_offset);

         enum class chunk_type {
            none,
            group,
            record,
         };

         struct opened_group : public group_header {
            size_t position = 0;
            size_t end      = 0;
         };

      protected:
         void _validate_record_signature() const;
         void _open_group_here(); // call after reading the GRUP signature
         bool _open_record_here(lu::four_cc signature); // call after reading the non-GRUP signature

      public:
         std::array<opened_group, 7> groups;
         struct {
            record_view    record;
            subrecord_view subrecord;
         } subviews;
         struct {
            bool assume_file_is_valid = false; // use when opening a new view to a file that's already been fully scanned and indexed (i.e. EVERY record and group has been read)

            bool allow_suspicious_record_signatures = false;
            bool allow_unknown_record_signatures    = false;
         } options;
      protected:
         const file*              _target_file = nullptr;
         lu::memory::basic_buffer _decompressed_record;
         bytereader               _reader;
         bare_form_id             _last_potential_group_parent = 0;
         
      public:
         constexpr const file* get_target_file() const noexcept { return this->_target_file; }
         void set_target_file(const file*);
         void set_position(size_t); // can throw std::out_of_range
         opened_group* get_current_group();
         chunk_type next_record_or_group();
   };
}
