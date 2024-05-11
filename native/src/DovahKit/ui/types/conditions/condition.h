#pragma once
#include "dovah/data/conditions/comparison_operator.h"
#include "dovah/data/conditions/run_on_type.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/components/conditions/working_parameter.h"
#include "dovah/forms/components/conditions.h"

namespace ui::types::conditions {
   using backend_condition_type = dovah::loaded_forms::components::condition;

   using condition = dovah::loaded_forms::components::conditions::working_condition;
   using context   = dovah::loaded_forms::components::conditions::context;
   using parameter = dovah::loaded_forms::components::conditions::working_parameter;

   using comparison_operator = dovah::conditions::comparison_operator;
   using run_on_type         = dovah::conditions::run_on_type;

   using parameter_underlying_type = dovah::conditions::parameter_underlying_type;
}