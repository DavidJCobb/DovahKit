#pragma once
#include <QString>
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions.h"

namespace editor_helpers {
   extern QString stringify_condition_argument(
      bool& incomplete_information,
      const dovah::loaded_forms::components::condition& cnd,
      int   arg_index,
      const dovah::loaded_forms::components::conditions::context&
   );
}