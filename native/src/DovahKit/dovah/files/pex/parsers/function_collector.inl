#pragma once
#include "./function_collector.h"

#include "./exceptions/bad_header_sentinel.h"
#include "./exceptions/bad_string_id.h"

namespace dovah::pex::parsers {
   constexpr const function_collector::function_info* function_collector::class_info::lookup_function(std::string_view name) const {
      for (auto& item : this->functions)
         if (dovah::papyrus::helpers::name_equals(item.name, name))
            return &item;
      return nullptr;
   }

   constexpr void function_collector::read_file(const char* buffer, size_t size) {
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
      this->_file_string_table.read(*this);
      {
         bool presence;
         this->read(presence);
         if (presence) {
            skip_bytes(sizeof(uint64_t));
            skip_length_prefixed_vector<2, function_debug_info>();
         }
      }
      {  // User flags
         uint16_t count;
         read(count);
         skip_bytes(3 * count);
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
         this->results.name = _retained_strings.get_or_insert(name);
         skip_bytes(4); // size
         _consume_and_retain_indexed_string(this->results.superclass);
         _skip_indexed_string(); // docstring
         
         skip_bytes(sizeof(script_object::user_flags));
         _skip_indexed_string(); // auto state name
         skip_length_prefixed_vector<2, typename decltype(script_object::variables)::value_type>();
         skip_length_prefixed_vector<2, typename decltype(script_object::properties)::value_type>();
         {  // script_object::states
            uint16_t count = 0;
            read(count);
            for (uint16_t i = 0; i < count; ++i) {
               _skip_indexed_string(); // state::name
               {  // script_object::states::functions
                  uint16_t count = 0;
                  read(count);
                  for (uint16_t i = 0; i < count; ++i) {
                     std::string_view name;
                     _consume_indexed_string(name);
                     if (this->results.lookup_function(name)) {
                        compiled_function::skip(*this);
                        continue;
                     }
                     auto& item = this->results.functions.emplace_back();
                     item.name = _retained_strings.get_or_insert(name);
                     _consume_and_retain_indexed_string(item.return_type);
                     _consume_and_retain_indexed_string(item.docstring);
                     skip_bytes(sizeof(compiled_function::flags));
                     {
                        decltype(compiled_function::function_flags) v;
                        read(v);
                        item.is_global = v & compiled_function::flag::global;
                        item.is_native = v & compiled_function::flag::native;
                     }
                     if (this->collect_arguments) {
                        read(item.arg_count);
                        item.arg_info.resize(item.arg_count);
                        for (uint16_t i = 0; i < item.arg_count; ++i) {
                           auto& arg = item.arg_info[i];
                           _consume_and_retain_indexed_string(arg.name);
                           _consume_and_retain_indexed_string(arg.type);
                        }
                     } else {
                        auto prior = this->get_position();
                        read(item.arg_count);
                        this->file._pos = prior;
                        skip_length_prefixed_vector<2, typename decltype(compiled_function::arguments)::value_type>();
                     }
                     skip_length_prefixed_vector<2, typename decltype(compiled_function::locals)::value_type>();
                     skip_length_prefixed_vector<2, typename decltype(compiled_function::opcodes)::value_type>();
                  }
               }
            }
         }
      }
   }

   constexpr const std::string_view function_collector::get_tabled_string(uint16_t index) const {
      return this->_file_string_table.lookup(index);
   }
}
