#include "./stringify_conditions.h"
#include "../core.h"
#include <QCoreApplication>
#include <QStringBuilder>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/form_stub.h"
#include "editor/helpers/condition_to_string/boolean_link.h"
#include "editor/helpers/condition_to_string/comparison_operand.h"
#include "editor/helpers/condition_to_string/comparison_operator.h"
#include "editor/helpers/condition_to_string/event_parameter.h"
#include "editor/helpers/condition_to_string/non_event_parameter.h"
#include "editor/helpers/condition_to_string/parameter_set.h"
#include "editor/helpers/condition_to_string/run_on.h"

namespace {
   namespace conditions {
      using namespace dovah::conditions;
      using namespace dovah::loaded_forms::components::conditions;
   }
   using condition = dovah::loaded_forms::components::condition;
   using context   = dovah::loaded_forms::components::conditions::context;

   constexpr const auto form_format = editor_helpers::condition_to_string::options::form_format{
      .form_type         = editor_helpers::condition_to_string::options::form_type_format::none,
      .include_editor_id = editor_helpers::condition_to_string::options::editor_id_presence::only_for_form,
      .include_form_id   = editor_helpers::condition_to_string::options::form_id_presence::if_no_editor_id,
      .include_placement = editor_helpers::condition_to_string::options::form_id_presence::never,
   };
}

namespace editor_helpers {
   extern QString stringify_condition(
      const dovah::loaded_forms::components::condition& cnd,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      auto& editor = DovahKitCore::get();
      auto& rod    = cnd.get_run_on_data();
      //
      out += condition_to_string::run_on(ctx, rod.type, rod.index, rod.reference.get_form_stub(), form_format).first;
      out += QCoreApplication::translate("condition to string", ".", "separator between run-on and function");
      //
      auto* func = cnd.get_function();
      if (func)
         out += QString::fromUtf8(QByteArray(func->name.data(), func->name.size()));
      else
         out += QCoreApplication::translate("condition to string", "?%1", "unknown condition function").arg(cnd.get_function_id());
      out += QCoreApplication::translate("condition to string", "(", "condition arg delimiter - open");
      out += condition_to_string::parameter_set(ctx, cnd, true, false, form_format);
      out += QCoreApplication::translate("condition to string", ")", "condition arg delimiter - close");
      //
      auto& cmp = cnd.get_comparison();
      out += QCoreApplication::translate("condition to string", " ", "separator between params and comparison operator");
      out += condition_to_string::comparison_operator(cmp.op);
      out += QCoreApplication::translate("condition to string", " ", "separator between comparison operator and operand");
      out += condition_to_string::comparison_operand(cmp);
      //
      return out;
   }
   extern QString stringify_condition(
      const dovah::loaded_forms::components::conditions::working_condition& cnd,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      auto& editor = DovahKitCore::get();

      {
         uint32_t          index = 0;
         dovah::form_stub* stub  = nullptr;
         if (std::holds_alternative<dovah::form_stub*>(cnd.run_on.entity)) {
            stub = std::get<dovah::form_stub*>(cnd.run_on.entity);
         } else if (std::holds_alternative<uint32_t>(cnd.run_on.entity)) {
            index = std::get<uint32_t>(cnd.run_on.entity);
         }
         out += condition_to_string::run_on(ctx, cnd.run_on.type, index, stub, form_format).first;
      }
      out += QCoreApplication::translate("condition to string", ".", "separator between run-on and function");

      const auto* func = dovah::conditions::function_info_by_id(cnd.function);
      if (func)
         out += QString::fromUtf8(QByteArray(func->name.data(), func->name.size()));
      else
         out += QCoreApplication::translate("condition to string", "?%1", "unknown condition function").arg(cnd.function);
      out += QCoreApplication::translate("condition to string", "(", "condition arg delimiter - open");
      out += condition_to_string::parameter_set(ctx, cnd, true, false, form_format);
      out += QCoreApplication::translate("condition to string", ")", "condition arg delimiter - close");
      //
      out += QCoreApplication::translate("condition to string", " ", "separator between params and comparison operator");
      out += condition_to_string::comparison_operator(cnd.comparison.op);
      out += QCoreApplication::translate("condition to string", " ", "separator between comparison operator and operand");
      out += condition_to_string::comparison_operand(cnd.comparison);
      return out;
   }

   extern QString stringify_condition_boolean_operator(
      const dovah::loaded_forms::components::condition& cnd
   ) {
      return condition_to_string::boolean_link(cnd.get_flags() & condition::flag::or_linked);
   }
   extern QString stringify_condition_boolean_operator(
      const dovah::loaded_forms::components::conditions::working_condition& cnd
   ) {
      return condition_to_string::boolean_link(cnd.flags.or_linked);
   }

   extern QString stringify_condition_list(
      const dovah::loaded_forms::components::condition_list& list,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      size_t  size = list.size();
      if (!size)
         return out;
      for (size_t i = 0; i < size - 1; ++i) {
         const auto& cnd = list[i];
         out += stringify_condition(cnd, ctx);
         out += " " % stringify_condition_boolean_operator(cnd) % " ";
      }
      if (size)
         out += stringify_condition(list[size - 1], ctx);
      //
      return out;
   }
}