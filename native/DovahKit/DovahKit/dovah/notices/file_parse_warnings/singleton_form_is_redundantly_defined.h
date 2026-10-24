#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class singleton_form_is_redundantly_defined final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr singleton_form_is_redundantly_defined(
            const std::string& overriding_file,
            bare_form_id       current_record_id,
            form_stub&         current_form_stub,
            form_stub&         previous_form_stub
         ) :
            base_file_parse_warning(overriding_file),
            previous_stub(previous_form_stub),
            current_stub(current_form_stub),
            current_record_id(current_record_id)
         {}

      public:
         form_stub&   previous_stub;
         form_stub&   current_stub;
         bare_form_id current_record_id;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"