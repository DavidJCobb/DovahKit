#include "./comparison_operand.h"
#include "dovah/forms/components/conditions/comparison_data.h"
#include "dovah/forms/components/conditions/working_comparison.h"
#include "dovah/form_stub.h"
#include "./form.h"

namespace editor_helpers::condition_to_string {
   extern QString comparison_operand(const dovah::loaded_forms::components::conditions::comparison_data& cmp, const options::form_format& format) {
      if (std::holds_alternative<dovah::form_reference_t>(cmp.operand)) {
         const auto* stub = std::get<dovah::form_reference_t>(cmp.operand).get_form_stub();
         return form(stub, format);
      } else {
         return comparison_operand(std::get<float>(cmp.operand));
      }
   }
   extern QString comparison_operand(const dovah::loaded_forms::components::conditions::working_comparison& cmp, const options::form_format& format) {
      if (std::holds_alternative<dovah::form_stub*>(cmp.operand)) {
         const auto* stub = std::get<dovah::form_stub*>(cmp.operand);
         return form(stub, format);
      } else {
         return comparison_operand(std::get<float>(cmp.operand));
      }
   }
   extern QString comparison_operand(float v) {
      return QString::number(v);
   }
}
