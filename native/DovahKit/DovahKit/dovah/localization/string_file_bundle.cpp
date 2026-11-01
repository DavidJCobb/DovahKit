#include "./string_file_bundle.h"
#include <limits>
#include "lu/bytestreams/base_bytereader.h"
#include "dovah/bsa/archive_collection.h"
#include "dovah/facts/localized_strings/all_language_names.h"
#include "dovah/facts/localized_strings/serialized_strings_are_length_prefixed.h"
#include "dovah/tes/file.h"

namespace dovah::localization {
   string_file_bundle::string_file_bundle(const tes::file& tes_file) : _tes_file(tes_file) {
   }

   bool string_file_bundle::id_exists(localized_string_type type, bare_localized_string_id id) const noexcept {
      auto& presence = this->_defined_ids.for_type(type);
      return presence.find(id) != presence.end();
   }

   void string_file_bundle::load(const dovah::bsa::archive_collection& archives) {
      std::string base_name = this->_tes_file.path().filename().string();
      {
         size_t i = base_name.find_last_of('.');
         if (i != std::string::npos)
            base_name = base_name.substr(0, i);
      }
      for (size_t i = 0; i < language_count; ++i) {
         auto& data = this->_by_language[i];

         std::string base_name_with_lang;
         {
            const auto& lang_name = facts::localized_strings::all_language_names[i];
            base_name_with_lang.reserve(base_name.size() + 1 + lang_name.size());
            base_name_with_lang += base_name;
            base_name_with_lang += '_';
            base_name_with_lang += lang_name;
         }

         {
            auto& dst      = data.general;
            auto  filename = base_name_with_lang + ".strings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::general, dst, this->_defined_ids.general);
         }
         {
            auto& dst      = data.description;
            auto  filename = base_name_with_lang + ".dlstrings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::description, dst, this->_defined_ids.description);
         }
         {
            auto& dst      = data.info;
            auto  filename = base_name_with_lang + ".ilstrings";
            dst.buffer = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!dst.buffer.empty())
               this->_parse_single_file(localized_string_type::info, dst, this->_defined_ids.info);
         }
      }
   }
   std::optional<string_file_bundle::const_string_view> string_file_bundle::lookup(localized_string_type type, language lang, bare_localized_string_id id) const noexcept {
      auto& file = this->_by_language[(size_t)lang].for_type(type);
      auto  it   = file.strings_by_id.find(id);
      if (it == file.strings_by_id.end())
         return {};
      const string_info& info = it->second;
      return const_string_view(
         (const char*)file.buffer.data_at(info.offset),
         info.size
      );
   }

   void string_file_bundle::_parse_single_file(localized_string_type type, string_file& file, id_set& presence) {
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

      const uint32_t text_data_base_offset = reader.position() + (count * 8);

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
            offset += text_data_base_offset;
            uint32_t length;
            if (offset + sizeof(length) > file.buffer.size())
               continue;
            length = *(uint32_t*)file.buffer.data_at(offset);
            offset += sizeof(length);
            if (offset + length > file.buffer.size()) // truncated string!
               continue;
            //
            file.strings_by_id[id] = { offset, length };
            presence.insert(id);
         }
      } else {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            if (!reader.read(id, offset))
               break;
            offset += text_data_base_offset;
            if (offset > file.buffer.size())
               break;
            const char*    start  = (const char*)file.buffer.data_at(offset);
            const uint32_t length = (uint32_t)strnlen_s(
               start,
               std::max(file.buffer.size() - offset, (size_t)std::numeric_limits<uint32_t>::max())
            );
            file.strings_by_id[id] = { offset, length };
            presence.insert(id);
         }
      }
   }
}