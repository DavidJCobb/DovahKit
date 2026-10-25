#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class game_setting_has_multiple_records_in_a_file final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr game_setting_has_multiple_records_in_a_file(
            const std::string& source_filename,
            std::string_view   setting_name,
            bare_form_id       current_record_id,
            bare_form_id       current_form_id,
            bare_form_id       previous_form_id
         ) :
            base_file_parse_warning(source_filename),
            setting_name(setting_name),
            current_record_id(current_record_id),
            current_form_id(current_form_id),
            previous_form_id(previous_form_id)
         {}

      public:
         std::string  setting_name;
         bare_form_id current_record_id;
         bare_form_id current_form_id;
         bare_form_id previous_form_id;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"