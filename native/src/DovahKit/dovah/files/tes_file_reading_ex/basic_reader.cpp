#include "basic_reader.h"
#include "../../notice_code_list.h"
extern "C" {
   #include "../../../zlib/zlib.h"
}

namespace dovah::tes_file_reading {
   #pragma region basic stream operations
   void basic_reader::_read_impl(void* buffer, uint32_t size) {
      assert(this->file_data);
      std::uintptr_t addr = (std::uintptr_t)this->file_data + this->stream_position;
      memcpy(buffer, (void*)addr, size);
      this->stream_position += size;
   }
   //
   void basic_reader::set_position(uint32_t pos) {
      this->stream_position = pos;
   }
   uint32_t basic_reader::get_position() const noexcept {
      return this->stream_position;
   }
   void basic_reader::skip(uint32_t count) {
      this->stream_position += count;
   }
   void basic_reader::rewind(uint32_t by) {
      if (this->stream_position <= by)
         this->stream_position = 0;
      else
         this->stream_position -= by;
   }
   bool basic_reader::is_available() const noexcept {
      return (this->file_data) && (this->file_size > 0);
   }
   bool basic_reader::is_good() const noexcept {
      return !this->is_in_bounds(1);
   }
   bool basic_reader::is_eof() const noexcept {
      return this->stream_position >= this->file_size;
   }
   #pragma endregion

   bool basic_reader::_reset_last_error() {
      this->last_error = detailed_notice();
      this->last_error.type = detailed_notice::notice_type::error;
   }
   bool basic_reader::_validate_record_signature() {
      this->_reset_last_error();
      //
      auto& record    = this->_record;
      auto  signature = record.signature();
      if (this->options.log_file_syntax_errors) {
         if (!this->options.allow_suspicious_record_signatures) {
            if (form_type_info::signature_is_suspicious(signature)) {
               this->last_error.code = notice_code::invalid_record_signature;
               this->last_error.set_file_offset(this->get_position());
               this->last_error.set_cause_signature(signature);
               //
               return false;
            }
         }
         if (!this->options.allow_unknown_record_signatures) {
            if (form_type_info::signature_to_form_type(signature) == form_type::none) {
               this->last_error.code = notice_code::invalid_record_signature;
               this->last_error.set_file_offset(this->get_position());
               this->last_error.set_cause_signature(signature);
               //
               return false;
            }
         }
      }
      return true;
   }
   basic_reader::object_type basic_reader::next_record_or_group() {
      auto& record = this->_record;
      if (record) {
         this->set_position(record.end);
         record.reset();
      }
      //
      if (!this->is_available())
         return object_type::none;
      //
      // Make sure we properly handle passing the end of a group:
      //
      auto pos = this->get_position();
      for (uint32_t i = 0; i < this->_groups.size(); i++) {
         auto& group = this->_groups[i];
         if (!group)
            break;
         if (pos >= group.end)
            group.reset();
      }
      //
      if (!this->is_good())
         return object_type::none;
      uint32_t signature;
      this->read(signature);
      signature = _byteswap_ulong(signature);
      if (signature == 'GRUP') {
         this->rewind(4);
         //
         auto pos = this->get_position();
         int32_t parent = -1;
         for (uint32_t i = 0; i < this->_groups.size(); i++) {
            auto& group = this->_groups[i];
            if (!group)
               break;
            parent = i;
         }
         assert(parent + 1 < this->_groups.size());
         auto& group = this->_groups[parent + 1];
         group.pos   = this->get_position();
         this->read(group.header);
         group.header.signature = _byteswap_ulong(group.header.signature);
         group.end   = group.pos + group.header.size;
         return object_type::group;
      }
      //
      // else it must be a record
      //
      this->rewind(4);
      //
      record.head_pos = this->get_position();
      if (auto& group = this->get_current_group())
         if (record.head_pos >= group.end)
            return object_type::none;
      this->read(record.header);
      record.header.signature = _byteswap_ulong(record.header.signature);
      record.body_pos = this->get_position();
      record.end = record.body_pos + record.header.size;
      if (!this->is_good())
         return object_type::none;
      if (!this->_validate_record_signature()) // also logs the appropriate error
         return object_type::none;
      {
         switch (record.header.signature) {
            case 'CELL':
            case 'DIAL':
            case 'WRLD':
               this->last_potential_group_parent = record.header.formID;
               break;
            default:
               this->last_potential_group_parent = 0;
         }
      }
      if (record.header.body_is_compressed()) {
         uint32_t decompressed_size;
         uint32_t compressed_size = record.header.size - sizeof(decompressed_size);
         this->read(decompressed_size);
         record.data.resize(decompressed_size);
         if (decompressed_size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.data.empty()) {
               this->last_error.code = notice_code::out_of_memory;
               this->last_error.set_file_offset(pos);
               this->last_error.set_cause_size(decompressed_size);
               return object_type::none;
            } else {
               auto input_buffer = malloc(compressed_size);
               this->read(input_buffer, compressed_size);
               uint32_t out_size = decompressed_size;
               uncompress((Bytef*)record.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
               free(input_buffer);
               if (out_size != decompressed_size) {
                  //dovah::logging::print_line("Size mismatch for decompressed record! Offset %08X, expected final size %08X, got size %08X.", record.head_pos, decompressed_size, out_size);
               }
               assert(out_size == decompressed_size);
            }
         }
      } else {
         record.data.resize(record.header.size);
         if (record.header.size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.data.empty()) {
               this->last_error.code = notice_code::out_of_memory;
               this->last_error.set_file_offset(pos);
               this->last_error.set_cause_size(record.header.size);
               return object_type::none;
            } else {
               this->read(record.data.raw(), record.header.size);
            }
         }
      }
      record.offset = 0;
      //
      return object_type::record;
   }
   //
   bool basic_reader::next_subrecord() {
      this->_reset_last_error();
      if (!this->is_available())
         return false;
      auto& r = this->_record;
      if (this->_subrecord.header.signature) {
         this->_record.skip(this->_subrecord.end - this->_record.stream_pos());
         this->_subrecord.header.signature = 0;
      }
      if (!this->_record.is_in_bounds())
         return false;
      uint16_t size;
      this->_record.read(this->_subrecord.header.signature);
      this->_record.read(size);
      this->_subrecord.header.size = size;
      if (this->_subrecord.header.signature == 'XXXX') {
         //
         // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
         // larger than what can be represented with the usual two-byte length.
         //
         if (this->_subrecord.header.size != 4) {
            this->last_error = detailed_notice();
            this->last_error.code = notice_code::extended_subrecord_with_no_length;
            this->last_error.set_file_offset(this->get_position());
            //
            this->_subrecord.header.signature = 0;
            return false;
         }
         static_assert(sizeof(this->_subrecord.header.size) == 4, "XXXX subrecords store a four-byte subrecord length. Alter the struct definition accordingly.");
         this->_record.read(this->_subrecord.header.size); // the contents of the XXXX subrecord are the length
         //
         // Get the next subrecord.
         //
         this->_record.read(this->_subrecord.header.signature);
         this->_record.skip(2);
      }
      this->_subrecord.header.signature = _byteswap_ulong(this->_subrecord.header.signature);
      this->_subrecord.pos = this->_record.body_pos + this->_record.offset;
      this->_subrecord.end = this->_subrecord.pos + size;
      if (!this->is_good() || !this->_record.is_in_bounds())
         return false;
      return true;
   }
}