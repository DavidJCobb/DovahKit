#pragma once
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"

namespace dovahscript::api_helpers::conditions {
   using context_type       = dovah::loaded_forms::components::conditions::context;
   using condition_type     = dovah::loaded_forms::components::condition;
   using working_type       = dovah::loaded_forms::components::conditions::working_condition;
   using working_comparison = dovah::loaded_forms::components::conditions::working_comparison;
   using working_parameter  = dovah::loaded_forms::components::conditions::working_parameter;

   using parameter_type_override = dovah::loaded_forms::components::conditions::parameter_type_override;
}