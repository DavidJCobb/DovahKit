#pragma once
#include <QString>
#include "../../dovah/forms/components/conditions.h"

namespace dovah::loaded_forms {
   namespace components {
      namespace condition_info {
         class arg_type;
      }
      class  condition_arg_value;
      struct condition;
   }
   class Package;
   class Quest;
}

namespace editor_helpers {
   extern QString stringify_condition_argument(
      bool& incomplete_information,
      const dovah::loaded_forms::components::condition& cnd,
      int   arg_index,
      const dovah::loaded_forms::components::condition_context&
   );
}