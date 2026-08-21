#pragma once
#include <QString>
#include "./options/form_format.h"
namespace dovah::loaded_forms::components::conditions {
   struct comparison_data;
   struct working_comparison;
}

namespace editor_helpers::condition_to_string {
   extern QString comparison_operand(const dovah::loaded_forms::components::conditions::comparison_data&,    const options::form_format& = {});
   extern QString comparison_operand(const dovah::loaded_forms::components::conditions::working_comparison&, const options::form_format& = {});
   extern QString comparison_operand(float);
}
