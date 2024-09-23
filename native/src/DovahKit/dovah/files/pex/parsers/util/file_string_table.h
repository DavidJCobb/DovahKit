#pragma once
#include <string_view>
#include <vector>

namespace dovah::pex::parsers::util {
   struct file_string_table {
      uint16_t              count = 0;
      std::string_view      data;
      std::vector<uint16_t> lengths;
      std::vector<size_t>   offsets;

      constexpr std::string_view lookup(size_t index) const {
         if (index >= this->count)
            return {};
         return this->data.substr(this->offsets[index], this->lengths[index]);
      }

      template<typename Parser>
      constexpr void read(Parser& parser) {
         parser.read(this->count);
         this->lengths.resize(this->count);
         this->offsets.resize(this->count);

         size_t table_start = parser.get_position();
         for (uint16_t i = 0; i < count; ++i) {
            auto& length = this->lengths[i];

            parser.read(length);
            this->offsets[i] = parser.get_position() - table_start;
            parser.skip_bytes(length);
         }
         size_t table_size = parser.get_position() - table_start;

         this->data = std::string_view((const char*)parser.get_buffer_data() + table_start, table_size);
      }
   };
}