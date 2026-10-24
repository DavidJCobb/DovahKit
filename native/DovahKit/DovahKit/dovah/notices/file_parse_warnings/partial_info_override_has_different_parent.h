#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   class partial_info_override_has_different_parent final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr partial_info_override_has_different_parent(
            const std::string& source_filename,
            bare_form_id       overriding_record_id,
            form_stub&         overridden_stub,
            form_stub*         parent_of_overriding,
            form_stub*         parent_of_overridden
         ) :
            base_file_parse_warning(source_filename),
            overriding_record_id(overriding_record_id),
            overridden_stub(overridden_stub),
            parent_of_overriding(parent_of_overriding),
            parent_of_overridden(parent_of_overridden)
         {}

      public:
         bare_form_id overriding_record_id;
         form_stub&   overridden_stub;
         form_stub*   parent_of_overriding = nullptr;
         form_stub*   parent_of_overridden = nullptr;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"