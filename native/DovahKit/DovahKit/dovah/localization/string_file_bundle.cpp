#include "./string_file_bundle.h"
#include <limits>
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
      auto& data = this->_by_language[(size_t)lang];
      auto& file = [&data, type]() -> const string_file& {
         switch (type) {
            case localized_string_type::general:
               return data.general;
            case localized_string_type::description:
               return data.description;
            case localized_string_type::info:
               return data.info;
         }
         std::unreachable();
      }();
      auto it = file.strings_by_id.find(id);
      if (it == file.strings_by_id.end())
         return {};
      const string_info& info = it->second;
      return const_string_view(
         (const char*)file.buffer.data_at(info.offset),
         info.size
      );
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
      file.strings_by_id[0] = { 0, 0 }; // by convention, ID 0 is an empty string
      if (facts::localized_strings::serialized_strings_are_length_prefixed(type)) {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            if (!reader.read(id, offset))
               break;
            //
            // The length is paired with the string data, not the header.
            //
            uint32_t length;
            if (offset + sizeof(length) > file.buffer.size())
               continue;
            length = *(uint32_t*)file.buffer.data_at(offset);
            offset += sizeof(length);
            if (offset + length > file.buffer.size()) // truncated string!
               continue;
            //
            file.strings_by_id[id] = { offset, length };
         }
      } else {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            if (!reader.read(id, offset))
               break;
            if (offset > file.buffer.size())
               break;
            const char*    start  = (const char*)file.buffer.data_at(offset);
            const uint32_t length = strnlen_s(
               start,
               std::max(file.buffer.size() - offset, (size_t)std::numeric_limits<uint32_t>::max())
            );
            file.strings_by_id[id] = { offset, length };
         }
      }
   }
}