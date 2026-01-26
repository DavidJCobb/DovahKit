#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class linked_refs_duplicate_keyword : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr linked_refs_duplicate_keyword(
            form_stub& subject,
            form_stub* keyword
         )
         :
            base_form_load_warning(subject),
            keyword(keyword)
         {}
         
         form_stub* keyword = nullptr;
   };
}
#include "../../../_util.undef.h"