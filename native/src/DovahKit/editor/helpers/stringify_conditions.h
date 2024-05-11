#pragma once
#include <QString>
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions.h"

namespace editor_helpers {
   // Stringify the content of a condition, not including the OR/AND linkage.
   extern QString stringify_condition(
      const dovah::loaded_forms::components::condition&,
      const dovah::loaded_forms::components::conditions::context&
   );

   extern QString stringify_condition_list(
      const dovah::loaded_forms::components::condition_list&,
      const dovah::loaded_forms::components::conditions::context&
   );
}