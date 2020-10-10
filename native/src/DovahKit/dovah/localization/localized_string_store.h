#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include "../files/bsa/bsa_archived_file.h"
#include "common.h"

namespace dovah {
   class localized_string_store {
      public:
         using id_t            = uint32_t;
         using language_name_t = std::string;
         using content_t       = std::string; // UTF-8
         using file_type = localized_string_type;
         //
         struct language_t {
            public:
               using entry_map_t = std::unordered_map<id_t, content_t>;
               //
               language_name_t name;
               bool loaded = false;
               struct {
                  entry_map_t common;
                  entry_map_t description;
                  entry_map_t info;
               } entries;
               //
            public:
               entry_map_t& entries_by_type(file_type);
               void load(localized_string_store& owner);
               //
            protected:
               void _load_single_file(localized_string_store& owner, file_type);
         };
         //
      protected:
         std::filesystem::path tes_filename; // name of the ES[LPM] file
         bsa_load_order&       file_source;
         std::unordered_map<language_name_t, language_t> content_by_language;
         //
      public:
         localized_string_store(bsa_load_order&, const std::filesystem::path& tes_filename);
         ~localized_string_store();
         //
         language_name_t default_language;
         //
         void open_language_files(const language_name_t&);
         const content_t& lookup(file_type, const language_name_t&, id_t);
         const content_t& lookup(file_type t, id_t i) {
            return this->lookup(t, this->default_language, i);
         }
   };
}
