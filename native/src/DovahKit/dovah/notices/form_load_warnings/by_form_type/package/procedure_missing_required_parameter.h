#pragma once
#include <cstdint>
#include <string>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class procedure_missing_required_parameter final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr procedure_missing_required_parameter(
            form_stub& subject,
            size_t param_index,
            std::string param_name
         )
         :
            base_form_load_warning(subject),
            param_index(param_index),
            param_name(param_name)
         {}

         size_t      param_index;
         std::string param_name;
   };
}
#include "../../../_util.undef.h"