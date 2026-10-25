#pragma once
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class game_setting_wants_to_override_a_real_form final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr game_setting_wants_to_override_a_real_form(
            const std::string& source_filename,
            std::string_view   setting_name,
            form_stub&         conflicting_form
         ) :
            base_file_parse_warning(source_filename),
            setting_name(setting_name),
            conflicting_form(conflicting_form)
         {}

      public:
         std::string setting_name;
         form_stub&  conflicting_form;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"