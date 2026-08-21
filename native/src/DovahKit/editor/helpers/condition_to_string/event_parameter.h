#pragma once
#include <cstdint>
#include <QString>
#include "./options/form_format.h"
namespace dovah::loaded_forms::components::conditions {
   struct context;
   struct event_parameters;
   struct working_event_parameters;
}

namespace editor_helpers::condition_to_string {
   extern QString event_function(uint16_t);
   extern QString event_member(
      const dovah::loaded_forms::components::conditions::context&,
      uint16_t
   );

   extern QString event_parameter(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::conditions::event_parameters&,
      size_t param,
      const options::form_format & = {}
   );
   extern QString event_parameter(
      const dovah::loaded_forms::components::conditions::context&,
      const dovah::loaded_forms::components::conditions::working_event_parameters&,
      size_t param,
      const options::form_format& = {}
   );
}
