#pragma once
#include <cstdint>
#include <QString>
#include "dovah/forms/components/conditions/working_parameter.h"
#include "./options/form_format.h"
namespace dovah {
   namespace conditions {
      struct parameter_typeinfo;
      enum class parameter_underlying_type;
   }
   namespace loaded_forms::components {
      namespace conditions {
         struct context;
         class  working_condition;
      }
      class condition;
   }
}

namespace editor_helpers::condition_to_string {
   extern QString non_event_parameter(
      const    dovah::loaded_forms::components::conditions::context&,
      uint16_t function_id,
      const    dovah::conditions::parameter_typeinfo*,
      dovah::conditions::parameter_underlying_type,
      const    dovah::loaded_forms::components::conditions::working_parameter&,
      size_t   which,
      bool     show_special_cases = true,
      const    options::form_format & = {}
   );

   extern QString non_event_parameter(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::conditions::working_condition&,
      size_t which,
      bool  show_special_cases = true,
      const options::form_format& = {}
   );
   extern QString non_event_parameter(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::condition&,
      size_t which,
      bool  show_special_cases = true,
      const options::form_format& = {}
   );
}
