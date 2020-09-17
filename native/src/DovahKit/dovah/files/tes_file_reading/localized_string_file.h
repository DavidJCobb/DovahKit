#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <map>

namespace dovah::tes_file_reading {
   class localized_string_file {
      //
      // TODO: REVISE THIS. We can't use a mapped_file, because these files might be stored in  
      // Interface.bsa. That also means that we need BSA-reading code in order to grab these 
      // at all.
      //
      public:
         enum class file_type {
            common,    // STRINGS
            journal,   // DLSTRINGS
            subtitles, // ILSTRINGS
         };
         using string_id_t   = uint32_t;
         using file_offset_t = uint32_t;
         enum class file_language : int8_t {
            unknown = -1,
            arabic  =  0,
            chinese,
            czech,
            danish,
            english,
            finnish,
            french,
            german,
            greek,
            hungarian,
            italian,
            japanese,
            norwegian,
            polish,
            portugese,
            russian,
            spanish,
            swedish,
            turkish,
         };
         enum class encoding : int8_t {
            unknown      = -1,
            utf8         =  0, // all East Asian languages; after Skyrim Classic, all non-English languages
            windows_1250, // Czech, Hungarian, Polish
            windows_1251, // Russian
            windows_1252, // all others
            windows_1253, // Greek
            windows_1254, // Turkish
            windows_1256, // Arabic
         };
         //
      public:
         localized_string_file() {}
         ~localized_string_file() {}
         //
         std::filesystem::path path;
         file_language language = file_language::unknown;
         file_type type        = file_type::common;
         uint32_t  count       = 0;
         uint32_t  buffer_size = 0;
         std::map<string_id_t, std::string> entries; // TODO: store an entry struct containing the string's encoding // not a vector; string IDs can be unordered in the file and, presumably, discontiguous
         //
      public:
         bool lookup(string_id_t id, std::string& out, encoding& out_encoding) const noexcept;
         bool open() { return false; }; // TODO: WRITE ME
         void set_path_from_owner_path(const std::filesystem::path&); // receives the path of an ES[LPM] file. you must set the language before calling.

         static encoding get_primary_language_encoding(file_language);
         static encoding get_secondary_language_encoding(file_language);
         static bool file_type_uses_length_prefixes(file_type);
   };
}