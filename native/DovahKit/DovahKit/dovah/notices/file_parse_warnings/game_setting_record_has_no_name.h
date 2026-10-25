#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class game_setting_record_has_no_name final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr game_setting_record_has_no_name(
            const std::string& source_filename,
            bare_form_id       record_id,
            bare_form_id       form_id // zero = absent
         ) :
            base_file_parse_warning(source_filename),
            record_id(record_id),
            form_id(form_id)
         {}

      public:
         bare_form_id form_id;
         bare_form_id record_id;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"