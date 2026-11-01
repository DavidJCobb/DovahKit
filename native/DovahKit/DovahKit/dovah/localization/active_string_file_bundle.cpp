#include "./active_string_file_bundle.h"
#include <limits>
#include "lu/bytestreams/base_bytereader.h"
#include "dovah/bsa/archive_collection.h"
#include "dovah/facts/localized_strings/all_language_names.h"
#include "dovah/facts/localized_strings/serialized_strings_are_length_prefixed.h"
#include "dovah/tes/file.h"
#include "dovah/types/asset_file_buffer.h"

namespace dovah::localization {
   active_string_file_bundle::active_string_file_bundle(const tes::file& tes_file) : _tes_file(tes_file) {
   }

   bool active_string_file_bundle::id_exists(localized_string_type type, bare_localized_string_id id) const noexcept {
      auto& presence = this->_defined_ids.for_type(type);
      return presence.find(id) != presence.end();
   }

   void active_string_file_bundle::load(const dovah::bsa::archive_collection& archives) {
      std::string base_name = this->_tes_file.path().filename().string();
      if (base_name.empty()) {
         //
         // This is a newly-/implicitly-created active file. It will not have any localized string 
         // content yet.
         //
         return;
      }
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
            auto  buffer   = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!buffer.empty())
               this->_parse_single_file(buffer, localized_string_type::general, dst, this->_defined_ids.general);
         }
         {
            auto& dst      = data.description;
            auto  filename = base_name_with_lang + ".dlstrings";
            auto  buffer = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!buffer.empty())
               this->_parse_single_file(buffer, localized_string_type::description, dst, this->_defined_ids.description);
         }
         {
            auto& dst      = data.info;
            auto  filename = base_name_with_lang + ".ilstrings";
            auto  buffer = archives.lookup_file(std::string("strings\\") + filename);
            dst.strings_by_id.clear();
            if (!buffer.empty())
               this->_parse_single_file(buffer, localized_string_type::info, dst, this->_defined_ids.info);
         }
      }
   }
   std::optional<active_string_file_bundle::const_string_view> active_string_file_bundle::lookup(localized_string_type type, language lang, bare_localized_string_id id) const noexcept {
      auto& file = this->_by_language[(size_t)lang].for_type(type);
      auto  it   = file.strings_by_id.find(id);
      if (it == file.strings_by_id.end())
         return {};
      const auto& s = it->second;
      return const_string_view(s.data(), s.size());
   }
   void active_string_file_bundle::replace(localized_string_type type, language lang, bare_localized_string_id id, const std::string& v) {
      if (id == 0) {
         return;
      }
      auto& file = this->_by_language[(size_t)lang].for_type(type);
      file.strings_by_id[id] = v;
   }

   void active_string_file_bundle::_parse_single_file(asset_file_buffer& buffer, localized_string_type type, string_file& file, id_set& presence) {
      lu::bytestreams::base_bytereader reader;
      reader.set_buffer({ buffer.data(), buffer.size() });
      if constexpr (std::endian::native != std::endian::little) {
         reader.set_needs_endianness_swap(true);
      }

      uint32_t count;
      if (!reader.read(count)) {
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
            if (offset + sizeof(length) > buffer.size())
               continue;
            length = *(uint32_t*)buffer.data_at(offset);
            offset += sizeof(length);
            if (offset + length > buffer.size()) // truncated string!
               continue;
            //
            presence.insert(id);
            file.strings_by_id[id].assign((const char*)buffer.data_at(offset), length);
         }
      } else {
         for (uint32_t i = 0; i < count; ++i) {
            bare_localized_string_id id;
            uint32_t                 offset = 0;
            if (!reader.read(id, offset))
               break;
            offset += text_data_base_offset;
            if (offset > buffer.size())
               break;
            const char*    start  = (const char*)buffer.data_at(offset);
            const uint32_t length = (uint32_t)strnlen_s(
               start,
               std::max(buffer.size() - offset, (size_t)std::numeric_limits<uint32_t>::max())
            );
            presence.insert(id);
            file.strings_by_id[id].assign((const char*)buffer.data_at(offset), length);
         }
      }
   }
}