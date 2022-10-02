#include "reader.h"
#include "notice_code_list.h"
//
#include "types/NiBound.h"
#include "types/NiColor.h"
#include "types/NiMatrix33.h"
#include "types/NiTransform.h"

#include <glm/glm.hpp>

namespace nifDK {

   #pragma region block_guard
   file_reader::block_guard::block_guard(file_reader& o, int32_t bi, size_t size, const std::string& block_type) : owner(o), block_index(bi) {
      assert(!o.is_in_block());
      if (!o.is_in_bounds(size)) {
         o.raise_error(notice_code::stream_ended_early);
      }
      o._block_index = bi;
      o._block_type  = block_type;
      std::swap(o.states.current, o.states.backup);
      o.states.current = {
         .data     = (void*)((std::intptr_t)o.states.backup.data + o.states.backup.position),
         .size     = size,
         .position = 0,
      };
   }
   file_reader::block_guard::~block_guard() {
      if (this->block_index == -1)
         return;
      assert(this->owner.is_in_block());
      assert(this->owner._block_index == this->block_index);
      this->owner._block_index = -1;
      std::swap(this->owner.states.current, this->owner.states.backup);
      this->owner.states.current.position += this->owner.states.backup.size; // skip to end of block
   }
   #pragma endregion

   void file_reader::read_endianness(file_passkey) {
      uint8_t v;
      this->read(v);
      switch (v) {
         case 0:
            this->endianness = std::endian::big;
            break;
         case 1:
            this->endianness = std::endian::little;
            break;
         default:
            this->throw_error(notice_code::unrecognized_endianness);
      }
   }
   void file_reader::read_string_table(file_passkey) {
      uint32_t string_count;
      //
      this->read(string_count);
      this->read(this->string_table.max_length);
      this->string_table.list.resize(string_count);
      for (auto& item : this->string_table.list)
         this->read_prefixed_string<uint32_t>(item);
   }

   int32_t file_reader::index_of_block(block* b) const {
      auto& list = this->subject->all_blocks;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == b)
            return i;
      return -1;
   }

   void file_reader::raise_error(const detailed_notice& c) {
      if (this->error.code != default_notice_code)
         return;
      this->error = c;
      this->error.type = detailed_notice::notice_type::error;
      //
      if (!this->error.offset)
         this->error.offset = this->file_position();
      this->error.flags |= detailed_notice::flag::has_file_offset;
      //
      if (this->error.cause.block.index == -1)
         this->error.cause.block.index = this->block_index();
      if (this->error.cause.block.index != -1)
         this->error.flags |= detailed_notice::flag::has_cause_block;
   }
   void file_reader::raise_error(notice_code_t c) {
      if (this->error.code != default_notice_code)
         return;
      this->error.type = detailed_notice::notice_type::error;
      this->error.code = c;
      //
      this->error.offset = this->file_position();
      this->error.flags |= detailed_notice::flag::has_file_offset;
      //
      this->error.cause.block.index = this->block_index();
      if (this->error.cause.block.index != -1)
         this->error.flags |= detailed_notice::flag::has_cause_block;
   }

   void file_reader::_read_ref(block*& out) {
      out = nullptr;
      if (!this->is_in_bounds(4))
         this->_on_read_failure();
      int32_t index;
      this->unchecked_read(index);
      if (index == -1) // sentinel for "None"
         return;
      //
      // NOTE: Refs should always point down the hierarchy; other types exist for back-references.
      //
      assert(this->subject && "You shouldn't be attempting to read refs except from a file's reader.");
      auto* instance = this->subject->block_by_index(index);
      if (instance == nullptr) {
         this->raise_error(notice_code::block_ref_has_invalid_index);
         throw read_error(this->error.code);
      }
      out = instance;
      return;
   }
   void file_reader::_on_read_failure() {
      this->raise_error(notice_code::stream_ended_early);
      throw read_error(this->error.code);
   }
   void file_reader::_on_read_ref_mismatch(const char* expected, const char* actual) {
      this->raise_error(notice_code::block_ref_has_incorrect_type);
      this->error.typenames.expected = expected;
      this->error.typenames.actual   = actual;
      throw read_error(this->error.code);
   }

   void file_reader::read_indexed_string(std::string& out) {
      if (this->version() <= file_version::from_parts<20, 0, 0, 5>) {
         this->read_prefixed_string<uint32_t>(out);
         return;
      }
      int32_t index;
      this->read(index);
      if (index < 0) {
         out.clear();
         return;
      }
      //
      auto& list = this->string_table.list;
      if (index >= list.size())
         this->throw_error(notice_code::string_index_out_of_bounds);
      out = list[index];
   }
   void file_reader::read_line_string(std::string& out) {
      uint8_t byte;
      bool    line = false;
      while (this->is_in_bounds(1)) {
         this->unchecked_read(byte);
         if (byte == '\n' || byte == '\00') {
            line = true;
            break;
         }
         if (byte == '\r') {
            line = true;
            continue;
         }
         if (line)
            this->throw_error(notice_code::line_string_with_bad_end); // '\r' not followed by '\n'
         out.push_back(byte);
      }
      if (!line)
         this->throw_error(notice_code::stream_ended_early); // no line or null terminator
   }
}