#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class game_setting_record_has_bad_form_id final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr game_setting_record_has_bad_form_id(
            const std::string& source_filename,
            std::string_view   setting_name,
            bare_form_id       record_id
         ) :
            base_file_parse_warning(source_filename),
            setting_name(setting_name),
            record_id(record_id)
         {}

      public:
         std::string  setting_name;
         bare_form_id record_id;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"