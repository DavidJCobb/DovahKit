#include "./file_view.h"
#include "zlib/zlib.h"
#include "dovah/form_type_info.h"
#include "dovah/utils/files/tes/signature_is_suspicious.h"
#include "../file.h"
#include "dovah/exceptions/file_parse_errors/groups_too_deeply_nested.h"
#include "dovah/exceptions/file_parse_errors/record_decompression_failed.h"
#include "dovah/exceptions/file_parse_errors/record_has_invalid_signature.h"

namespace {
   namespace file_parse_errors {
      using namespace dovah::exceptions::file_parse_errors;
   }
}

namespace dovah::tes {
   file_view::file_view() : subviews(*this, *this) {
   }

   /*static*/ file_view file_view::open_view_to_group(const file& file, size_t group_header_offset) {
      file_view out;
      out.set_target_file(&file);
      out.set_position(group_header_offset);
      auto r = out.next_record_or_group();
      assert(r == chunk_type::group);
      return out;
   }
   /*static*/ file_view file_view::open_view_to_record(const file& file, size_t record_header_offset) {
      file_view out;
      out.set_target_file(&file);
      out.set_position(record_header_offset);
      auto r = out.next_record_or_group();
      assert(r == chunk_type::record);
      return out;
   }

   void file_view::_validate_record_signature() const {
      if (this->options.assume_file_is_valid)
         return;

      const auto& record    = this->subviews.record;
      const auto  signature = record.signature();

      if (!this->options.allow_suspicious_record_signatures) {
         if (utils::files::tes::signature_is_suspicious(signature)) {
            throw file_parse_errors::record_has_invalid_signature(
               *this->_target_file,
               record.file_offset_of_header(),
               record._header
            );
         }
      }
      if (!this->options.allow_unknown_record_signatures) {
         auto ft = form_type_info::signature_to_form_type(signature);
         if (ft == form_type::none) {
            throw file_parse_errors::record_has_invalid_signature(
               *this->_target_file,
               record.file_offset_of_header(),
               record._header
            );
         }
         if (this->_target_file->game() == game::skyrim_classic) {
            const auto& info = form_type_info::lookup(ft);
            if (info.flags & form_type_info::flag::is_skyrim_special) {
               throw file_parse_errors::record_has_invalid_signature(
                  *this->_target_file,
                  record.file_offset_of_header(),
                  record._header
               );
            }
         }
      }
   }

   void file_view::_open_group_here() {
      const size_t position_of_new_group = this->_reader.position() - sizeof(lu::four_cc);

      int32_t parent = -1;
      for (uint32_t i = 0; i < this->groups.size(); i++) {
         auto& group = this->groups[i];
         if (!group.signature)
            break;
         parent = i;
      }
      if (parent + 1 >= this->groups.size()) {
         std::vector<size_t> group_offsets;
         group_offsets.reserve(this->groups.size() + 1);
         for (auto& group : this->groups)
            group_offsets.push_back(group.position);
         group_offsets.push_back(position_of_new_group);
         throw file_parse_errors::groups_too_deeply_nested(*this->_target_file, std::move(group_offsets));
      }
      auto& group = this->groups[parent + 1];
      group.position  = position_of_new_group;
      group.signature = 'GRUP';
      this->_reader.read(
         group.size,
         group.label,
         group.type,
         group.version_control,
         group.unknown
      );
      group.end = group.position + group.size;
   }

   bool file_view::_open_record_here(lu::four_cc signature) {
      auto& record = this->subviews.record;
      record.file_offsets.head = (record_view::size_type)(this->_reader.position() - sizeof(lu::four_cc));
      if (auto* group = this->get_current_group())
         if (record.file_offsets.head >= group->end)
            return false;
      if (!this->_reader.is_in_bounds(lu::bytestreams::serialized_bytecount<decltype(this->_reader), record_header>::value - sizeof(lu::four_cc)))
         return false;
      record._header.signature = signature;
      this->_reader.read(
         record._header.size,
         record._header.flags,
         record._header.formID,
         record._header.version_control,
         record._header.version,
         record._header.version_control_2
      );
      record.file_offsets.body = (record_view::size_type)this->_reader.position();
      record.file_offsets.end  = record.file_offsets.body + record._header.size;
      if (!this->_reader.is_in_bounds()) {
         record.set_buffer({});
         return false;
      }
      this->_validate_record_signature(); // potentially throws
      {
         switch (record._header.signature) {
            case 'CELL':
            case 'DIAL':
            case 'WRLD':
               this->_last_potential_group_parent = record._header.formID;
               break;
            default:
               this->_last_potential_group_parent = 0;
         }
      }
      if (record._header.flags.has_all_of<record_flag::compressed>()) {
         uint32_t decompressed_size = 0;
         uint32_t compressed_size   = record._header.size - sizeof(decompressed_size);
         this->_reader.read(decompressed_size);
         this->_decompressed_record.clear();
         try {
            this->_decompressed_record.resize(decompressed_size);
         } catch (std::bad_alloc&) {
            throw file_parse_errors::record_decompression_failed(
               *this->_target_file,
               record.file_offsets.head,
               record._header,
               file_parse_errors::record_decompression_failed::failure_type::claimed_size_is_too_huge_to_even_try
            );
         }
         this->subviews.record.set_buffer({ this->_decompressed_record.data(), this->_decompressed_record.size() });
         if (decompressed_size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            auto input_buffer = malloc(compressed_size);
            this->_reader.read_bytes(input_buffer, compressed_size);
            uint32_t out_size = decompressed_size;
            auto result = uncompress((Bytef*)this->_decompressed_record.data(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
            free(input_buffer);
            if (this->options.assume_file_is_valid) {
               assert(result == Z_OK);
               assert(out_size == decompressed_size);
            } else {
               using error_type = file_parse_errors::record_decompression_failed::failure_type;

               if (result != Z_OK) {
                  auto problem = error_type::zlib_unknown_error;
                  switch (result) {
                     case Z_BUF_ERROR:
                        problem = error_type::data_larger_than_expected;
                        break;
                     case Z_MEM_ERROR:
                        problem = error_type::zlib_memory_error;
                        break;
                     case Z_DATA_ERROR:
                        problem = error_type::data_corrupt_or_incomplete;
                        break;
                  }
                  throw file_parse_errors::record_decompression_failed(
                     *this->_target_file,
                     record.file_offsets.head,
                     record._header,
                     problem
                  );
               }
               if (out_size != decompressed_size) {
                  throw file_parse_errors::record_decompression_failed(
                     *this->_target_file,
                     record.file_offsets.head,
                     record._header,
                     error_type::uncompressed_data_is_not_of_declared_size
                  );
               }
            }

            if (out_size != decompressed_size) {
               //dovah::logging::print_line("Size mismatch for decompressed record! Offset %08X, expected final size %08X, got size %08X.", record.head_pos, decompressed_size, out_size);
            }
            assert(out_size == decompressed_size);
         }
      } else {
         //
         // The record body is uncompressed. Just have the record-view read directly 
         // out of the mapped file data.
         //
         this->subviews.record.set_buffer({ this->_reader.data_at_position(), record._header.size});
      }
      return true;
   }

   void file_view::_reset_parse_state() {
      auto& record    = this->subviews.record;
      auto& subrecord = this->subviews.subrecord;

      record.set_buffer({});
      record._header      = {};
      record.file_offsets = {};
      subrecord.set_buffer({});
      subrecord._body_pos  = 0;
      subrecord._signature = 0;
      this->groups = {};
      this->_decompressed_record.clear();
      this->_last_potential_group_parent = 0;
   }

   void file_view::set_target_file(const file* file) {
      this->_target_file = file;
      if (file) {
         this->_reader.set_buffer({ file->data(), file->size() });

         const bool is_opposite_endianness = this->_target_file->app_flags().has_all_of<file_app_specific_flag::is_opposite_endianness>();
         this->_reader.set_needs_endianness_swap(is_opposite_endianness);
         this->subviews.record.set_needs_endianness_swap(is_opposite_endianness);
         this->subviews.subrecord.set_needs_endianness_swap(is_opposite_endianness);
      } else {
         this->_reader.set_buffer({});
      }
      this->_reset_parse_state();
   }
   void file_view::set_position(size_t p) {
      this->_reader.seek(p);
      this->_reset_parse_state();
   }

   file_view::opened_group* file_view::parent_of_group(const opened_group& g) {
      auto* data = this->groups.data();
      auto  i    = &g - data;
      if (i)
         return &data[i - 1];
      return nullptr;
   }
   file_view::opened_group* file_view::get_current_group() {
      if (!this->groups[0].signature)
         return nullptr;
      size_t i = 1;
      for (; i < this->groups.size(); ++i)
         if (!this->groups[i].signature)
            return &this->groups[i - 1];
      return &this->groups[i - 1];
   }
   void file_view::skip_current_group() {
      auto* group = this->get_current_group();
      if (!group)
         return;
      auto pos = group->end;
      this->_reader.seek(pos);

      size_t i = 0;
      for (; i < this->groups.size(); ++i) {
         auto& group = this->groups[i];
         //
         // Shenanigans with control flow: we know (but the compiler won't) that 
         // the group list represents nested groups: if we've reached or exceeded 
         // the end-position of an outer group, then we must necessarily also have 
         // reached or exceeded the end-position of an inner group (unless the 
         // file is malformed). Thus we only need to compare positions until we 
         // find a group we've reached or passed the end of, and then we can just 
         // blindly close all opened groups thereafter.
         //
         if (!group.signature)
            break;
         if (pos >= group.end) {
            do {
               this->groups[i] = {};
            } while ((++i < this->groups.size()) && this->groups[i].signature);
            break;
         }
      }
   }

   file_view::chunk_type file_view::next_record_or_group() {
      if (!this->_target_file)
         return chunk_type::none;

      auto& record = this->subviews.record;
      if (record.exists()) {
         this->_reader.unchecked_seek(record.file_offset_of_end());
         record._header = {};
         record.file_offsets = {};

         auto& subrecord = this->subviews.subrecord;
         if (subrecord.exists()) {
            subrecord._signature = 0;
            subrecord._body_pos  = 0;
            subrecord.set_buffer({});
         }
      }
      //
      // Make sure we properly handle passing the end of a group, and account for 
      // nested groups having the same endpoint:
      //
      {
         const auto position = this->_reader.position();
         size_t i = 0;
         for (; i < this->groups.size(); ++i) {
            auto& group = this->groups[i];
            //
            // Shenanigans with control flow: we know (but the compiler won't) that 
            // the group list represents nested groups: if we've reached or exceeded 
            // the end-position of an outer group, then we must necessarily also have 
            // reached or exceeded the end-position of an inner group (unless the 
            // file is malformed). Thus we only need to compare positions until we 
            // find a group we've reached or passed the end of, and then we can just 
            // blindly close all opened groups thereafter.
            //
            if (!group.signature)
               break;
            if (position >= group.end) {
               do {
                  this->groups[i] = {};
               } while ((++i < this->groups.size()) && this->groups[i].signature);
               break;
            }
         }
      }
      if (!this->_reader.is_in_bounds()) // bounds-check the earlier seek operation
         return chunk_type::none;

      lu::four_cc signature;
      if (!this->_reader.read(signature))
         return chunk_type::none;
      if (signature == 'GRUP') {
         this->_open_group_here();
         return chunk_type::group;
      }
      if (!this->_open_record_here(signature))
         return chunk_type::none;
      return chunk_type::record;
   }
}