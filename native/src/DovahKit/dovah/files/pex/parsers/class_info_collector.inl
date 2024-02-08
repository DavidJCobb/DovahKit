#pragma once
#include "./class_info_collector.h"

#include "./exceptions/bad_header_sentinel.h"
#include "./exceptions/bad_string_id.h"

namespace dovah::pex::parsers {
   constexpr void class_info_collector::read_file(const char* buffer, size_t size) {
      this->set_buffer(buffer, size);
      {
         uint32_t magic;
         read(magic);
         if (magic != 0xFA57C0DE) {
            if (magic != std::byteswap(0xFA57C0DE))
               throw exceptions::bad_header_sentinel(*this);
            this->_needs_endian_swap = true;
         }
      }
      {  // Read file header.
         game_id game = (game_id)0;
         skip_bytes(
            sizeof(decltype(file_header::version)::major) +
            sizeof(decltype(file_header::version)::minor)
         );
         read(game);
         if (game != game_id::skyrim) {
            // TODO: throw: wrong game.
         }
         skip_bytes(sizeof(file_header::compile_time));
         skip_length_prefixed_string<2>();
         skip_length_prefixed_string<2>();
         skip_length_prefixed_string<2>();
      }
      {
         uint16_t count;
         read(count);

         size_t offset = this->get_position();
         for (uint16_t i = 0; i < count; ++i)
            skip_length_prefixed_string<2>();
         size_t length = this->get_position() - offset;

         this->_file_string_table = std::string_view((const char*)this->file._buffer + offset, length);
      }
      {
         bool presence;
         this->read(presence);
         if (presence) {
            skip_bytes(sizeof(uint64_t));
            skip_length_prefixed_vector<2, function_debug_info>();
         }
      }
      {  // User flags
         auto& dfn_cnd = this->user_flags.conditional;
         auto& dfn_hid = this->user_flags.hidden;

         uint8_t found_both = 0;

         uint16_t count;
         read(count);

         uint16_t i = 0;
         for (; i < count; ++i) {
            if (dfn_cnd != 0xFF && dfn_hid != 0xFF)
               break;

            uint16_t index;
            uint8_t  bit;
            read(index);
            read(bit);

            auto view = get_tabled_string(index);
            if (dfn_cnd == 0xFF) {
               if (dovah::papyrus::helpers::name_equals(view, "conditional")) {
                  dfn_cnd = bit;
                  continue;
               }
            }
            if (dfn_hid == 0xFF) {
               if (dovah::papyrus::helpers::name_equals(view, "hidden")) {
                  dfn_hid = bit;
                  continue;
               }
            }
         }
         if (i < count)
            skip_bytes(3 * (count - i));
      }

      uint16_t object_count;
      read(object_count);

      bool is_not_first_match = false;
      for(size_t i = 0; i < object_count; ++i) {
         auto prior = this->get_position();

         std::string_view name;
         _consume_indexed_string(name);

         if (!dovah::papyrus::helpers::name_equals(name, desired_classname)) {
            this->file._pos = prior;
            script_object::skip(*this);
            continue;
         }

         if (!is_not_first_match) {
            this->results = {};
         }
         is_not_first_match = false;
         this->results.name = name;
         skip_bytes(4); // size
         _consume_indexed_string(this->results.superclass);
         _consume_indexed_string(this->results.docstring);
         
         uint32_t user_flags = 0;
         read(user_flags);
         if (user_flags & (1 << this->user_flags.conditional)) {
            this->results.flags.conditional = true;
         }
         if (user_flags & (1 << this->user_flags.hidden)) {
            this->results.flags.hidden = true;
         }

         this->file._pos = prior;
         script_object::skip(*this);
      }
   }

   constexpr const std::string_view class_info_collector::get_tabled_string(uint16_t index) const {
      const auto& src = this->_file_string_table;

      size_t size = src.size();
      size_t pos  = 0;
      for (size_t i = 0; i < index; ++i) {
         uint16_t length = *(uint16_t*)&src[pos];
         if (this->_needs_endian_swap)
            length = std::byteswap(length);
         pos += sizeof(length);
         pos += length;
      }

      uint16_t length = *(uint16_t*)&src[pos];
      if (this->_needs_endian_swap)
         length = std::byteswap(length);
      pos += sizeof(length);
      return std::string_view(src.data() + pos, length);
   }
}
