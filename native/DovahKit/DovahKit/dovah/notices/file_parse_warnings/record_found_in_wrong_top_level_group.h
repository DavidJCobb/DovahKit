#pragma once
#include <cstdint>
#include <optional>
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   enum class form_type : uint8_t;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class record_found_in_wrong_top_level_group final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr record_found_in_wrong_top_level_group(
            const std::string&          source_filename,
            form_type                   expected_form_type,
            form_type                   actual_form_type,
            bare_form_id                record_id,
            std::optional<bare_form_id> form_id = {}
         ) :
            base_file_parse_warning(source_filename),
            expected_form_type(expected_form_type),
            actual_form_type(actual_form_type),
            record_id(record_id),
            form_id(form_id)
         {}

      public:
         form_type    expected_form_type;
         form_type    actual_form_type;
         bare_form_id record_id;
         std::optional<bare_form_id> form_id;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"