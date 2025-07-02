#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class procedure_typename_unrecognized final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr procedure_typename_unrecognized(
            form_stub& subject,
            std::string_view type
         )
         :
            base_form_load_warning(subject),
            type(type)
         {}

         std::string type;
   };
}
#include "../../../_util.undef.h"