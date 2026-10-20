#include "./record_view.h"
#include "./file_view.h"
#include "../file.h"
#include "dovah/exceptions/file_parse_errors/extended_subrecord_marker_at_eof.h"
#include "dovah/exceptions/file_parse_errors/extended_subrecord_marker_is_invalid.h"
#include "dovah/exceptions/file_parse_errors/truncated_subrecord.h"

namespace {
   namespace file_parse_errors {
      using namespace dovah::exceptions::file_parse_errors;
   }
}

namespace dovah::tes {
   subrecord_view& record_view::next_subrecord() {
      auto& subrecord = this->owner.subviews.subrecord;
      if (subrecord) {
         auto pos = this->owner.subviews.subrecord.end_pos_in_record();
         if (pos >= this->size()) {
            subrecord._signature = 0;
            subrecord._body_pos  = this->size();
            subrecord.set_buffer({});
            return subrecord;
         }
         this->unchecked_seek(pos);
      }
      if (!this->is_in_bounds(6))
         return subrecord;

      uint32_t subrecord_size = 0;
      {
         uint16_t narrow_size = 0;
         this->unchecked_read(
            subrecord._signature,
            narrow_size
         );
         if (subrecord._signature != 'XXXX') {
            subrecord_size = narrow_size;
         } else {
            //
            // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
            // larger than what can be represented with the usual two-byte length.
            //
            if (!this->owner.options.assume_file_is_valid) {
               if (narrow_size != sizeof(subrecord_size)) {
                  throw file_parse_errors::extended_subrecord_marker_is_invalid(
                     *this->owner.get_target_file(),
                     this->file_offsets.head,
                     this->header,
                     this->position() - 6
                  );
               }
            }
            if (!this->is_in_bounds(sizeof(subrecord_size) + sizeof(subrecord._signature) + sizeof(narrow_size))) {
               throw file_parse_errors::extended_subrecord_marker_at_eof(
                  *this->owner.get_target_file(),
                  this->file_offsets.head,
                  this->header,
                  this->position() - 6
               );
            }
            this->unchecked_read(
               subrecord_size,
               subrecord._signature
            );
            this->skip_bytes(sizeof(narrow_size));
         }
      }
      if (!this->is_in_bounds(subrecord_size)) {
         throw file_parse_errors::truncated_subrecord(
            *this->owner.get_target_file(),
            this->file_offsets.head,
            this->header,
            this->position() - 12,
            subrecord._signature,
            subrecord_size,
            this->size() - (size_type)this->position()
         );
      }
      subrecord._body_pos = (subrecord_view::size_type)this->position();
      subrecord.set_buffer({ this->data_at_position(), subrecord_size });
      return subrecord;
   }
   lu::four_cc record_view::peek_next_subrecord_signature() {
      auto pos = this->owner.subviews.subrecord.end_pos_in_record();
      if (pos + 4 < this->header.size) {
         auto prior = this->position();
         this->unchecked_seek(pos);
         lu::four_cc v;
         this->unchecked_read(v);
         this->unchecked_seek(prior);
         return v;
      }
      return 0;
   }
}