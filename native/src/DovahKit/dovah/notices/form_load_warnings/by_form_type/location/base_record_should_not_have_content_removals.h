#pragma once
#include "./base_contents_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::location {
   class base_record_should_not_have_content_removals : public base_contents_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr base_record_should_not_have_content_removals(
            form_stub& stub,
            contents_type type
         )
         :
            base_contents_warning(stub),
            type(type)
         {}

         contents_type type;
   };
}
#include "../../../_util.undef.h"