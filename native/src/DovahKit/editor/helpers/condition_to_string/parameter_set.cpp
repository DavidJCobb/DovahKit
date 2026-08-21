#include "./parameter_set.h"
#include <QCoreApplication>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/components/conditions.h"
#include "./event_parameter.h"
#include "./non_event_parameter.h"

namespace editor_helpers::condition_to_string {
   extern QString parameter_set(
      const dovah::loaded_forms::components::conditions::context& context,
      const dovah::loaded_forms::components::conditions::working_condition& cnd,
      bool  show_special_cases,
      bool  parenthesize_event,
      const options::form_format& form_format
   ) {
      const auto* function_info = dovah::conditions::function_info_by_id(cnd.function);
      if (!function_info)
         return {};

      if (function_info->uses_event_data) {
         const auto& params_opt = cnd.event_parameters;
         if (!params_opt.has_value()) {
            return {};
         }
         const auto&   params  = params_opt.value();
         const QString value_a = event_parameter(context, params, 0);
         const QString value_b = event_parameter(context, params, 1);
         const QString value_c = event_parameter(context, params, 2, form_format);

         if (parenthesize_event) {
            if (value_c.isEmpty()) {
               return QCoreApplication::translate("condition parameters", "%1(%2)", "event, 1, parenthesized").arg(value_a).arg(value_b);
            }
            return QCoreApplication::translate("condition parameters", "%1(%2, %3)", "event, 2, parenthesized").arg(value_a).arg(value_b).arg(value_c);
         }
         if (value_c.isEmpty()) {
            return QCoreApplication::translate("condition parameters", "%1 %2 ", "event, 1").arg(value_a).arg(value_b);
         }
         return QCoreApplication::translate("condition parameters", "%1 %2 %3", "event, 2").arg(value_a).arg(value_b).arg(value_c);
      }

      if (function_info->argument_types[0] != &dovah::conditions::parameter_types::None) {
         const QString value_a = non_event_parameter(context, cnd, 0, true, form_format);
         if (function_info->argument_types[1] && function_info->argument_types[1] != &dovah::conditions::parameter_types::None) {
            const QString value_b = non_event_parameter(context, cnd, 1, true, form_format);
            return QCoreApplication::translate("condition parameters", "%1, %2", "normal, 2").arg(value_a).arg(value_b);
         }
         return value_a;
      }

      return {};
   }
   extern QString parameter_set(
      const dovah::loaded_forms::components::conditions::context& context,
      const dovah::loaded_forms::components::condition& cnd,
      bool  show_special_cases,
      bool  parenthesize_event,
      const options::form_format& form_format
   ) {
      const auto* function_info = dovah::conditions::function_info_by_id(cnd.get_function_id());
      if (!function_info)
         return {};

      if (function_info->uses_event_data) {
         const auto& params = cnd.get_event_parameters();
         const QString value_a = event_parameter(context, params, 0);
         const QString value_b = event_parameter(context, params, 1);
         const QString value_c = event_parameter(context, params, 2, form_format);
         if (parenthesize_event) {
            if (value_c.isEmpty()) {
               return QCoreApplication::translate("condition parameters", "%1(%2)", "event, 1, parenthesized").arg(value_a).arg(value_b);
            }
            return QCoreApplication::translate("condition parameters", "%1(%2, %3)", "event, 2, parenthesized").arg(value_a).arg(value_b).arg(value_c);
         }
         if (value_c.isEmpty()) {
            return QCoreApplication::translate("condition parameters", "%1 %2 ", "event, 1").arg(value_a).arg(value_b);
         }
         return QCoreApplication::translate("condition parameters", "%1 %2 %3", "event, 2").arg(value_a).arg(value_b).arg(value_c);
      }

      if (function_info->argument_types[0] != &dovah::conditions::parameter_types::None) {
         const QString value_a = non_event_parameter(context, cnd, 0, true, form_format);
         if (function_info->argument_types[1] && function_info->argument_types[1] != &dovah::conditions::parameter_types::None) {
            const QString value_b = non_event_parameter(context, cnd, 1, true, form_format);
            return QCoreApplication::translate("condition parameters", "%1, %2", "normal, 2").arg(value_a).arg(value_b);
         }
         return value_a;
      }

      return {};
   }
}
