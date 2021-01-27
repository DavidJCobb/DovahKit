#pragma once
#include <QString>
#include "../../dovah/forms/components/conditions/arg_types.h"

namespace dovah::loaded_forms {
   namespace components {
      namespace condition_info {
         class arg_type;
      }
      class condition_arg_value;
   }
   class Package;
   class Quest;
}

namespace editor_helpers {
   extern QString stringify_condition_argument(
      bool& incomplete_information,
      const dovah::loaded_forms::components::condition_info::arg_type& type,
      const dovah::loaded_forms::components::condition_arg_value& value,
      const dovah::loaded_forms::Package* owning_package = nullptr,
      const dovah::loaded_forms::Quest* owning_quest = nullptr
   );
}