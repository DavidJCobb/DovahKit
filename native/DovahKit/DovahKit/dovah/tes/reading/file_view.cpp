#include "./file_view.h"
#include "../file.h"

namespace dovah::tes {
   bool file_view::_is_eof() const {
      return (this->position >= this->target_file->size());
   }
   void file_view::_read_bytes(void* dst, size_t dst_size) {
      memcpy(dst, this->target_file->data_at(this->position), dst_size);
      this->position += dst_size;
   }

   void file_view::_open_group_here() {
      int32_t parent = -1;
      for (uint32_t i = 0; i < this->groups.size(); i++) {
         auto& group = this->groups[i];
         if (!group.signature)
            break;
         parent = i;
      }
      assert(parent + 1 < this->groups.size());
      auto& group = this->groups[parent + 1];
      group.position  = this->position - sizeof(lu::four_cc);
      group.signature = 'GRUP';
      this->_read_bytes((char*)&group + sizeof(lu::four_cc), sizeof(group_header) - sizeof(lu::four_cc));
      if (this->target_file->header().needs_endianness_swap) {
         group.size            = std::byteswap(group.size);
         group.label           = std::byteswap(group.label);
         group.type            = (group_type)std::byteswap((std::underlying_type_t<group_type>)group.type);
         group.version_control = std::byteswap(group.version_control);
         group.unknown         = std::byteswap(group.unknown);
      }
      group.end = group.position + group.size;
   }

   bool file_view::_open_record_here(lu::four_cc signature) {
      auto& record = this->subviews.record;
      record.head_pos = this->position - sizeof(lu::four_cc);
      if (auto* group = this->get_current_group())
         if (record.head_pos >= group->end)
            return false;
      record.header.signature = signature;
      this->_read_bytes((char*)&record.header + sizeof(lu::four_cc), sizeof(record_header) - sizeof(lu::four_cc));
      if (this->target_file->header().needs_endianness_swap) {
         auto& header = record.header;
         header.size              = std::byteswap(header.size);
         header.flags             = record_flags::from_mask(std::byteswap((uint32_t)header.flags));
         header.formID            = std::byteswap(header.formID);
         header.version_control   = std::byteswap(header.version_control);
         header.version           = std::byteswap(header.version);
         header.version_control_2 = std::byteswap(header.version_control_2);
      }
      record.body_pos = this->position;
      record.end     = record.body_pos + record.header.size;
      if (this->_is_eof())
         return false;
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
         record.buffer.resize(decompressed_size);
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
            auto result = uncompress((Bytef*)record.buffer.data(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
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
         record.buffer.resize(record.header.size);
         if (record.header.size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.buffer.empty()) {
               auto error = std::make_unique<dovah::notices::file_load_errors::record_is_too_large>();
               _throw_file_read_error(*this, error);
            } else {
               this->read(record.buffer.data(), record.header.size);
            }
         }
      }
      record.offset = 0;
      return true;
   }

   file_view::opened_group* file_view::get_current_group();
   file_view::chunk_type file_view::next_record_or_group() {
      auto& record = this->subviews.record;
      if (record) {
         this->position = record.end;
         record.reset();
      }

      if (!this->target_file)
         return chunk_type::none;

      //
      // Make sure we properly handle passing the end of a group:
      //
      for (uint32_t i = 0; i < this->groups.size(); i++) {
         auto& group = this->groups[i];
         if (!group.signature)
            break;
         if (this->position >= group.end)
            group = {};
      }

      if (this->_is_eof())
         return chunk_type::none;

      lu::four_cc signature;
      {
         signature.value = *(uint32_t*)this->target_file->data_at(this->position);
         this->position += 4;
         if (!this->target_file->header().needs_endianness_swap) { // invert; signatures have opposite endianness to everything else
            signature.value = std::byteswap(signature.value);
         }
      }
      if (signature == 'GRUP') {
         this->_open_group_here();
         return chunk_type::group;
      }
      //
      // else it must be a record
      //
      if (!this->_open_record_here(signature))
         return chunk_type::none;
      return chunk_type::record;
   }
}