#include "basic_reader.h"
#include "file_loader.h"
extern "C" {
   #include "../../../zlib/zlib.h"
}

#include "../../exceptions/file_load_failed.h"
#include "../../notices/file_load_errors/extended_subrecord_marker_is_invalid.h"
#include "../../notices/file_load_errors/record_decompression_failed.h"
#include "../../notices/file_load_errors/record_has_invalid_signature.h"
#include "../../notices/file_load_errors/record_is_too_large.h"

namespace {
   template<typename T> requires (std::is_base_of_v<dovah::notices::base_file_load_error, T>)
   void _throw_file_read_error(dovah::tes_file_reading::basic_reader& self, std::unique_ptr<T>& err) {
      // TODO: CAN'T REPORT THE FAILING FILENAME; FIX THIS WHEN REDESIGNING FILE LOADING POST-LAUNCH
      //       (if `basic_reader` were a view into a single authoritative file, we'd be able to get that file's name)
      if (err->file_offset == 0)
         err->file_offset = self.get_position();

      if constexpr (std::is_base_of_v<dovah::notices::file_load_errors::base_record_load_error, T>) {
         const auto& record = self.get_current_record();
         err->record = {
            .signature     = record.signature(),
            .local_form_id = record.formID(),
            // TODO: CAN'T REPORT THE RESOLVED FORM ID; WE'RE TOO LOW-LEVEL TO KNOW IF WE'RE OPENING THIS SUBRECORD AT THE BEHEST OF A FORM STUB
            //       (perhaps as part of a post-launch rewrite, views into TES files could be given metadata e.g. the "asking" form stub's form ID?)
         };
         if constexpr (std::is_base_of_v<dovah::notices::file_load_errors::base_subrecord_load_error, T>) {
            const auto& subrecord = self.get_current_subrecord();
            err->subrecord = {
               .signature = subrecord.signature(),
               .size      = subrecord.size(),

               .offset_into_record = record.current_offset() - subrecord.offset(),
            };
         }
      }

      auto ex = dovah::exceptions::file_load_failed();
      ex.details.file_load_error = std::move(err);
      throw ex;
   }
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
   bool basic_reader::is_eof() const noexcept {
      return this->stream_position >= this->file_size;
   }
   #pragma endregion

   void basic_reader::_validate_record_signature() {
      auto& record    = this->_record;
      auto  signature = record.signature();
      if (!this->options.log_file_syntax_errors)
         return;

      if (!this->options.allow_suspicious_record_signatures) {
         if (form_type_info::signature_is_suspicious(signature)) {
            auto error = std::make_unique<dovah::notices::file_load_errors::record_has_invalid_signature>();
            _throw_file_read_error(*this, error);
         }
      }
      if (!this->options.allow_unknown_record_signatures) {
         if (form_type_info::signature_to_form_type(signature) == form_type::none) {
            auto error = std::make_unique<dovah::notices::file_load_errors::record_has_invalid_signature>();
            _throw_file_read_error(*this, error);
         }
      }
      return;
   }
   bool basic_reader::load_record_at(uint32_t pos) {
      this->reset_parse_state();
      this->set_position(pos);
      return this->next_record_or_group() == object_type::record;
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
      if (this->is_eof())
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
      if (this->is_eof())
         return object_type::none;
      this->_validate_record_signature(); // potentially throws
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
            if (this->options.log_file_syntax_errors) {
               assert(!record.data.empty());
            } else {
               using error_type = dovah::notices::file_load_errors::record_decompression_failed;

               auto error = std::make_unique<error_type>(error_type::problem_code::claimed_size_is_too_huge_to_even_try);
               _throw_file_read_error(*this, error);
            }

            auto input_buffer = malloc(compressed_size);
            this->read(input_buffer, compressed_size);
            uint32_t out_size = decompressed_size;
            auto result = uncompress((Bytef*)record.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
            free(input_buffer);

            if (this->options.log_file_syntax_errors) {
               using error_type = dovah::notices::file_load_errors::record_decompression_failed;

               if (result != Z_OK) {
                  auto problem = error_type::problem_code::zlib_unknown_error;
                  switch (result) {
                     case Z_BUF_ERROR:
                        problem = error_type::problem_code::data_larger_than_expected;
                        break;
                     case Z_MEM_ERROR:
                        problem = error_type::problem_code::zlib_memory_error;
                        break;
                     case Z_DATA_ERROR:
                        problem = error_type::problem_code::data_corrupt_or_incomplete;
                        break;
                  }

                  auto error = std::make_unique<error_type>(problem);
                  _throw_file_read_error(*this, error);
               }
               if (out_size != decompressed_size) {
                  auto error = std::make_unique<error_type>(error_type::problem_code::uncompressed_data_is_not_of_declared_size);
                  _throw_file_read_error(*this, error);
               }
            } else {
               assert(result == Z_OK);
               assert(out_size == decompressed_size);
            }

            if (out_size != decompressed_size) {
               //dovah::logging::print_line("Size mismatch for decompressed record! Offset %08X, expected final size %08X, got size %08X.", record.head_pos, decompressed_size, out_size);
            }
            assert(out_size == decompressed_size);
         }
      } else {
         record.data.resize(record.header.size);
         if (record.header.size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.data.empty()) {
               auto error = std::make_unique<dovah::notices::file_load_errors::record_is_too_large>();
               _throw_file_read_error(*this, error);
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
      if (!this->is_available())
         return false;
      auto& r = this->_record;
      if (this->_subrecord.header.signature) {
         this->_record.skip(this->_subrecord.end_pos() - this->_record.offset);
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
         bool marker_is_valid = this->_subrecord.header.size == 4;
         if (this->options.log_file_syntax_errors) {
            if (!marker_is_valid) {
               this->_subrecord.body_start = this->_record.offset - sizeof(size); // since `_throw_file_read_error` will want this info

               auto error = std::make_unique<dovah::notices::file_load_errors::extended_subrecord_marker_is_invalid>();
               _throw_file_read_error(*this, error);
            }
         } else {
            assert(marker_is_valid);
         }
         static_assert(sizeof(this->_subrecord.header.size) == 4, "XXXX subrecords store a four-byte subrecord length. Alter the struct definition accordingly.");
         this->_record.read(this->_subrecord.header.size); // the contents of the XXXX subrecord are the length
         //
         // Get the next subrecord.
         //
         this->_record.read(this->_subrecord.header.signature);
         this->_record.skip(2);
      }
      this->_subrecord.body_start = this->_record.offset;
      this->_subrecord.header.signature = _byteswap_ulong(this->_subrecord.header.signature);
      if (this->is_eof() || !this->_record.is_in_bounds())
         return false;
      return true;
   }

   bool basic_reader::uses_string_table() const noexcept {
      if (this->loader)
         return this->loader->uses_string_table();
      return this->options.uses_string_table;
   }
}