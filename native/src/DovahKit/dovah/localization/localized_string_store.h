#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include "../files/bsa/bsa_archived_file.h"
#include "common.h"

namespace dovah {
   class localized_string_store {
      public:
         using id_t            = uint32_t;
         using language_name_t = std::string;

         using const_string_view = std::basic_string_view<const char>; // UTF-8

      protected:
         struct string_info {
            uint32_t offset; // within `string_file::buffer` i.e. the string file
            uint32_t size;
         };

         struct string_file {
            std::unique_ptr<bsa_archived_file>    file;
            std::unordered_map<id_t, string_info> strings_by_id;
         };

         struct language_content {
            public:
               language_name_t name;
               bool loaded = false;
               struct {
                  string_file description;
                  string_file general;
                  string_file info;
               } files_by_type;
                
            public:
               void load(localized_string_store& owner);
                
            protected:
               void _load_single_file(localized_string_store& owner, localized_string_type);
         };
         
      protected:
         std::filesystem::path tes_filename; // name of the ES[LPM] file
         bsa_load_order&       file_source;
         std::unordered_map<language_name_t, language_content> content_by_language;
         language_name_t       default_language;
         localization_language default_language_enum = localization_language::unknown;
         
      public:
         localized_string_store(bsa_load_order&, const std::filesystem::path& tes_filename);
         ~localized_string_store();
         
         void set_default_language(const language_name_t&);
         inline const language_name_t& get_default_language() const noexcept { return this->default_language; }
         inline localization_language get_default_language_enum() const noexcept { return this->default_language_enum; }
         
         void open_language_files(const language_name_t&);
         const_string_view lookup(localized_string_type, const language_name_t&, id_t);
         const_string_view lookup(localized_string_type t, id_t i) {
            return this->lookup(t, this->default_language, i);
         }
   };
}
