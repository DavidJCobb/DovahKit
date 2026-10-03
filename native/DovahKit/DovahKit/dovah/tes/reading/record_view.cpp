#include "./record_view.h"
#include "./file_view.h"
#include "../file.h"

namespace dovah::tes {
   bool record_view::_needs_endianness_swap() const noexcept {
      return this->owner.target_file->header().needs_endianness_swap;
   }

   const subrecord_view& record_view::get_current_subrecord() const noexcept {
      return this->owner.subviews.subrecord;
   }
   subrecord_view& record_view::get_current_subrecord() noexcept {
      return this->owner.subviews.subrecord;
   }
   subrecord_view& record_view::next_subrecord() {
      auto& subrecord = this->owner.subviews.subrecord;
      if (subrecord) {
         auto pos = this->owner.subviews.subrecord.end_pos_in_record();
         if (pos >= this->size()) {
            subrecord._signature = 0;
            subrecord._size = 0;
            subrecord._body_pos = this->size();
            return subrecord;
         }
         this->offset = pos;
      }
      if (!this->is_in_bounds(6))
         return subrecord;
      uint16_t size;
      this->unchecked_read(subrecord._signature);
      this->unchecked_read(size);
      subrecord._size = size;
      if (subrecord._signature == 'XXXX') {
         //
         // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
         // larger than what can be represented with the usual two-byte length.
         //
         bool marker_is_valid = size == 4;
         if (this->options.log_file_syntax_errors) {
            if (!marker_is_valid) {
               this->_subrecord.body_start = this->_record.offset - sizeof(size); // since `_throw_file_read_error` will want this info

               auto error = std::make_unique<dovah::notices::file_load_errors::extended_subrecord_marker_is_invalid>();
               _throw_file_read_error(*this, error);
            }
         }
         static_assert(sizeof(subrecord._size) == 4, "XXXX subrecords store a four-byte subrecord length. Alter the struct definition accordingly.");
         if (!this->is_in_bounds(sizeof(subrecord._size) + sizeof(subrecord._signature) + sizeof(size))) {
            _throw_file_read_error(*this, error);
         }
         this->unchecked_read(subrecord._size);
         this->unchecked_read(subrecord._signature);
         this->skip_bytes(2);
      }
      subrecord._body_pos  = this->offset;
      return subrecord;
   }
   lu::four_cc record_view::peek_next_subrecord_signature() {
      auto pos = this->owner.subviews.subrecord.end_pos_in_record();
      if (pos < this->end) {
         lu::four_cc v;
         v.value = *(lu::four_cc::integral_type*)this->data_at(pos);
         if (!_needs_endianness_swap()) // negate; signatures use the opposite endianness of everything else
            v.value = std::byteswap(v.value);
         return v;
      }
      return 0;
   }
}