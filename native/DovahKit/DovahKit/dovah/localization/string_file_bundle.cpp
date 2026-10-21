#include "./string_file_bundle.h"
#include "lu/bytestreams/base_bytereader.h"
#include "dovah/bsa/archive_collection.h"
#include "dovah/facts/localized_strings/serialized_strings_are_length_prefixed.h"

namespace dovah::localization {
   string_file_bundle::string_file_bundle(const std::string& tes_filename) {
      this->_tes_filename = tes_filename;
   }

   void string_file_bundle::load(const dovah::bsa::archive_collection& archives) {
      if (this->_tes_filename.empty())
         return;
      std::string base_name = this->_tes_filename;
      {
         size_t i = base_name.find_last_of('.');
         if (i != std::string::npos)
            base_name = base_name.substr(0, i);
      }
      for (size_t i = 0; i < language_count; ++i) {
         auto& data = this->_by_language[i];
         {
            auto& dst      = data.general;
            auto  filename = base_name + ".strings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + base_name);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::general, dst);
         }
         {
            auto& dst      = data.description;
            auto  filename = base_name + ".dlstrings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + base_name);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::description, dst);
         }
         {
            auto& dst      = data.info;
            auto  filename = base_name + ".ilstrings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + base_name);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::info, dst);
         }
      }
   }
   std::optional<string_file_bundle::const_string_view> string_file_bundle::lookup(localized_string_type type, language lang, bare_localized_string_id id) const noexcept {
      auto& data    = this->_by_language[(size_t)lang];
      auto& strings = [&data, type]() -> const id_to_string_map& {
         switch (type) {
            case localized_string_type::general:
               return data.general.strings_by_id;
            case localized_string_type::description:
               return data.description.strings_by_id;
            case localized_string_type::info:
               return data.info.strings_by_id;
         }
         std::unreachable();
      }();
      auto it = strings.find(id);
      if (it == strings.end())
         return {};
      return const_string_view(it->second.data(), it->second.size());
   }

   void string_file_bundle::_parse_single_file(localized_string_type type, string_file& file) {
      lu::bytestreams::base_bytereader reader;
      reader.set_buffer({ file.buffer.data(), file.buffer.size() });
      if constexpr (std::endian::native != std::endian::little) {
         reader.set_needs_endianness_swap(true);
      }

      uint32_t count;
      if (!reader.read(count)) {
         file.buffer = {};
         return;
      }
      reader.skip_bytes(4); // size of all string data
      if (facts::localized_strings::serialized_strings_are_length_prefixed(type)) {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            uint32_t                 length;
            if (!reader.read(id, offset, length))
               break;
            if (offset > file.buffer.size())
               break;
            if (offset + length > file.buffer.size()) // truncated string!
               break;
            const char* start = (const char*)file.buffer.data_at(offset);
            file.strings_by_id[id] = { start, length };
         }
      } else {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            if (!reader.read(id, offset))
               break;
            if (offset > file.buffer.size())
               break;
            const char*  start  = (const char*)file.buffer.data_at(offset);
            const size_t length = strnlen_s(start, file.buffer.size() - offset);
            file.strings_by_id[id] = { start, length };
         }
      }
   }
}