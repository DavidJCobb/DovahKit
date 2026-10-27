#include "./managed_localized_string_use.h"
#include "dovah/tes/file.h"
#include "dovah/tes/file_header_flags.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah {
   bool _base_managed_localized_string_use::stream(tes::subrecord_view& subrecord) {
      const auto& source_file = subrecord.get_source_file();
      const bool  localized   = source_file.header().flags.has_all_of<tes::file_header_flag::localized_strings>();
      if (localized) {
         if (!std::holds_alternative<_localized_id>(this->_value))
            this->_value.emplace<_localized_id>();
         auto& v = std::get<_localized_id>(this->_value);
         if (subrecord.read(v.id)) {
            v.file = &source_file;
            return true;
         } else {
            return false;
         }
      } else {
         if (!std::holds_alternative<std::string>(this->_value))
            this->_value.emplace<std::string>();
         auto& v = std::get<std::string>(this->_value);
         subrecord.read_all_to_string(v);
         return true;
      }
   }
}