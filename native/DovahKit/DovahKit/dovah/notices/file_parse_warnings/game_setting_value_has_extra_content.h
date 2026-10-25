#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class game_setting_value_has_extra_content final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr game_setting_value_has_extra_content(
            const std::string& source_filename,
            std::string_view   setting_name,
            bare_form_id       record_id,
            size_t             expected_data_size,
            size_t             actual_data_size
         ) :
            base_file_parse_warning(source_filename),
            setting_name(setting_name),
            record_id(record_id),
            expected_data_size(expected_data_size),
            actual_data_size(actual_data_size)
         {}

      public:
         std::string  setting_name;
         bare_form_id record_id;
         size_t       expected_data_size;
         size_t       actual_data_size;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"