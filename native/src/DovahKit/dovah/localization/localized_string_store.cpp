#include "localized_string_store.h"
#include "../files/bsa/bsa_load_order.h"
#include "../../helpers/unordered_map.h"

namespace {
   static dovah::localized_string_store::content_t _missing_string = "<MISSING STRING>";
}

namespace dovah {
   localized_string_store::localized_string_store(bsa_load_order& blo, const std::filesystem::path& tfn) : file_source(blo), tes_filename(tfn) {
   }
   localized_string_store::~localized_string_store() {
   }

   localized_string_store::language_t::entry_map_t& localized_string_store::language_t::entries_by_type(file_type type) {
      switch (type) {
         case file_type::common:      return entries.common;
         case file_type::description: return entries.description;
         case file_type::info:        return entries.info;
      }
      __assume(0); // unreachable
   }
   void localized_string_store::language_t::_load_single_file(localized_string_store& owner, file_type type) {
      auto filename = owner.tes_filename;
      auto bare     = filename.stem().string();
      bare += '_';
      bare += this->name;
      switch (type) {
         case file_type::common:      bare += ".strings";   break;
         case file_type::description: bare += ".dlstrings"; break;
         case file_type::info:        bare += ".ilstrings"; break;
         default:
            assert(false);
      }
      filename.replace_filename(bare);
      filename = std::filesystem::path(L"strings") / filename;
      //
      auto* file = owner.file_source.lookup_file(filename.string(), true);
      if (!file)
         return;
      //
      uint32_t count;
      file->read(0, count); // 00: number of strings
      // 04: size of all string data
      //
      constexpr size_t size_of_header = 8;
      constexpr size_t size_of_entry  = 8;
      auto&  entries     = this->entries_by_type(type);
      size_t data_offset = size_of_header + (count * size_of_entry);
      entries[0] = "";
      for (uint32_t i = 0; i < count; ++i) {
         id_t     id;
         uint32_t offset;
         file->read(size_of_header + (size_of_entry * i) + 0,          id);
         file->read(size_of_header + (size_of_entry * i) + sizeof(id), offset);
         //
         auto& string = entries[id];
         offset += data_offset;
         if (type != file_type::common) {
            uint32_t size;
            file->read(offset, size);
            offset += sizeof(size);
            //
            if (size) {
               string.resize(size - 1);
               file->read(offset, (void*)string.data(), size - 1);
            }
         } else {
            char c;
            do {
               file->read(offset++, c);
               if (c)
                  string += c;
            } while (c);
         }
      }
   }
   void localized_string_store::language_t::load(localized_string_store& owner) {
      if (this->loaded)
         return;
      this->_load_single_file(owner, file_type::common);
      this->_load_single_file(owner, file_type::description);
      this->_load_single_file(owner, file_type::info);
      this->loaded = true;
   }

   void localized_string_store::open_language_files(const language_name_t& language_name) {
      if (this->tes_filename.empty())
         return;
      //
      auto normalized_language = language_name;
      for (auto& c : normalized_language)
         c = tolower(c);
      //
      auto& language = this->content_by_language[normalized_language];
      if (!language.loaded) {
         language.name = normalized_language;
         language.load(*this);
      }
   }
   const localized_string_store::content_t& localized_string_store::lookup(file_type type, const language_name_t& language_name, id_t id) {
      auto normalized_language = language_name;
      for (auto& c : normalized_language)
         c = tolower(c);
      if (!cobb::unordered_map_contains(this->content_by_language, normalized_language)) {
         //
         // TODO: should we load the language on demand?
         //
         return _missing_string; // for now, let's not
      }
      auto& language = this->content_by_language[normalized_language];
      auto& entries  = language.entries_by_type(type);
      if (!cobb::unordered_map_contains(entries, id))
         return _missing_string;
      return entries[id];
   }
}