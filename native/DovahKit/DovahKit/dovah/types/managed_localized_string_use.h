#pragma once
#include <concepts>
#include <string>
#include <variant>
#include "./bare_localized_string_id.h"
#include "dovah/tes/file_header_flags.h"
namespace dovah::tes {
   class file;
   class subrecord_view;
}

namespace dovah {
   template<auto LStringType>
   class managed_localized_string_use {
      protected:
         struct _localized_id {
            const tes::file*         file = nullptr; // TES file from which this string use is loaded
            bare_localized_string_id id   = 0;       // ID within the corresponding string file
         };

      protected:
         std::variant<
            std::string,
            _localized_id
         > _value;

      public:
         constexpr const bool is_localized() const noexcept {
            return std::holds_alternative<std::string>(this->_value);
         }

         template<
            // template trickery used here so that every header that includes this class doesn't also have 
            // to include the headers for these particular types
            std::same_as<tes::file> FileType = tes::file,
            std::same_as<tes::subrecord_view> SubrecordViewType = tes::subrecord_view
         >
         bool stream(SubrecordViewType& subrecord) {
            const FileType& source_file = subrecord.get_source_file();
            const bool      localized   = source_file.header().flags.has_all_of<tes::file_header_flag::localized_strings>();
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
   };
}