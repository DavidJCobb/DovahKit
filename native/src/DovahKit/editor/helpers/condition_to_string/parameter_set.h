#pragma once
#include <QString>
#include "./options/form_format.h"
namespace dovah::loaded_forms::components {
   namespace conditions {
      struct context;
      class  working_condition;
   }
   class condition;
}

namespace editor_helpers::condition_to_string {
   extern QString parameter_set(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::conditions::working_condition&,
      bool  show_special_cases = true,
      bool  parenthesize_event = false,
      const options::form_format& = {}
   );
   extern QString parameter_set(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::condition&,
      bool  show_special_cases = true,
      bool  parenthesize_event = false,
      const options::form_format& = {}
   );
}
