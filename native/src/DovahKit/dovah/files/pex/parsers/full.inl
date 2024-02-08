#pragma once
#include "./full.h"

#include "dovah/data/papyrus/helpers/name_equals.h"

#include "./exceptions/bad_header_sentinel.h"
#include "./exceptions/bad_string_id.h"

namespace dovah::pex::parsers {
   constexpr const std::string_view full::get_tabled_string(uint16_t index) const {
      if (index >= this->_string_table.size())
         throw exceptions::bad_string_id(*this, index);
      return this->_string_table[index];
   }
   
   constexpr void full::read_file(const uint8_t* buffer, size_t size) {
      this->set_buffer(buffer, size);
      {
         uint32_t magic;
         this->read(magic);
         if (magic != 0xFA57C0DE) {
            if (magic != std::byteswap(0xFA57C0DE))
               throw exceptions::bad_header_sentinel(*this);
            this->_needs_endian_swap = true;
         }
      }
      this->read(header);
      {
         uint16_t count;
         this->read(count);
         this->_string_table.resize(count);
         for (auto& s : this->_string_table)
            this->read_length_prefixed_string<2>(s);
      }
      {
         bool presence;
         this->read(presence);
         if (presence) {
            auto& data = this->debug.emplace();
            this->read(data.modification_time);
            this->read_length_prefixed_vector<2>(data.per_function);
         }
      }
      this->read_length_prefixed_vector<2>(this->user_flags);
      this->read_length_prefixed_vector<2>(this->objects);
   }

   constexpr const script_object* full::lookup_object(const std::string& name) const {
      for (auto it = this->objects.rbegin(); it != this->objects.rend(); ++it) {
         const auto& item = *it;
         if (dovah::papyrus::helpers::name_equals(item.name, name))
            return &item;
      }
      return nullptr;
   }
}