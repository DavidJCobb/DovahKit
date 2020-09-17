#include "localized_string_file.h"

namespace dovah::tes_file_reading {
   bool localized_string_file::lookup(string_id_t id, std::string& out, encoding& out_encoding) const noexcept {
      out.clear();
      out_encoding = encoding::unknown;
      //
      auto it = this->entries.find(id);
      if (it != this->entries.end()) {
         out = it->second;
         //
         // TODO: set (out_encoding) based on whether the text was valid in the primary encoding 
         // or had to use the secondary encoding. This is something we should probably determine 
         // at load time.
         //
         return true;
      }
      return false;
   }
   void localized_string_file::set_path_from_owner_path(const std::filesystem::path& file) {
      this->path = file;
      auto name = file.filename().wstring();
      name += '_';
      switch (this->language) {
         case file_language::arabic:    name += L"arabic"; break;
         case file_language::chinese:   name += L"chinese"; break;
         case file_language::czech:     name += L"czech"; break;
         case file_language::danish:    name += L"danish"; break;
         case file_language::english:   name += L"english"; break;
         case file_language::finnish:   name += L"finnish"; break;
         case file_language::french:    name += L"french"; break;
         case file_language::german:    name += L"german"; break;
         case file_language::greek:     name += L"greek"; break;
         case file_language::hungarian: name += L"hungarian"; break;
         case file_language::italian:   name += L"italian"; break;
         case file_language::japanese:  name += L"japanese"; break;
         case file_language::norwegian: name += L"norwegian"; break;
         case file_language::polish:    name += L"polish"; break;
         case file_language::portugese: name += L"portugese"; break;
         case file_language::russian:   name += L"russian"; break;
         case file_language::spanish:   name += L"spanish"; break;
         case file_language::swedish:   name += L"swedish"; break;
         case file_language::turkish:   name += L"turkish"; break;
         default:
            name += L"UNKNOWN"; break;
      }
      this->path.replace_filename(name);
   }
   //
   /*static*/ localized_string_file::encoding localized_string_file::get_primary_language_encoding(file_language l) {
      if (l == file_language::czech)
         return get_secondary_language_encoding(l);
      return encoding::utf8;
   }
   /*static*/ localized_string_file::encoding localized_string_file::get_secondary_language_encoding(file_language l) {
      switch (l) {
         case file_language::arabic: // unofficial translation or doesn't use localized string files? this encoding is used for module files
            return encoding::windows_1256;
            //
         case file_language::chinese:  // unofficial translation or doesn't use localized string files? this encoding is used for module files
         case file_language::japanese: // no secondary encoding
            return encoding::utf8;
            //
         case file_language::czech:
         case file_language::hungarian: // unofficial translation or doesn't use localized string files? this encoding is used for module files
         case file_language::polish:
            return encoding::windows_1250;
            //
         case file_language::greek: // unofficial translation or doesn't use localized string files? this encoding is used for module files
            return encoding::windows_1253;
            //
         case file_language::russian:
            return encoding::windows_1251;
            //
         case file_language::turkish: // unofficial translation or doesn't use localized string files? this encoding is used for module files
            return encoding::windows_1254;
            //
         default:
            return encoding::windows_1252;
      }
   }
   /*static*/ bool localized_string_file::file_type_uses_length_prefixes(file_type t) {
      switch (t) {
         case file_type::journal:
         case file_type::subtitles:
            return true;
      }
      return false;
   }
}