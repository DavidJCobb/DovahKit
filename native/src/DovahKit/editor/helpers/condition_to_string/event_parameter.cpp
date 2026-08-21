#include "./event_parameter.h"
#include <QCoreApplication>
#include "dovah/data/conditions/event_function.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/event_parameters.h"
#include "dovah/forms/components/conditions/working_event_parameters.h"
#include "dovah/forms/Quest.h"
#include "./form.h"

namespace editor_helpers::condition_to_string {
   extern QString event_function(uint16_t function_id) {
      switch (function_id) {
         case dovah::conditions::event_function::GetIsID:
            return "GetIsID";
         case dovah::conditions::event_function::GetItemValue:
            return "GetItemValue";
         case dovah::conditions::event_function::GetValue:
            return "GetValue";
         case dovah::conditions::event_function::HasKeyword:
            return "HasKeyword";
         case dovah::conditions::event_function::IsInList:
            return "IsInList";
      }
      return QCoreApplication::translate("condition event function, unknown ID", "<event function:%1>").arg(function_id);
   }
   extern QString event_member(
      const dovah::loaded_forms::components::conditions::context& context,
      uint16_t member
   ) {
      if constexpr (std::endian::native == std::endian::little) { // this really should be done within the condition internals...
         member = std::byteswap(member);
         // Dovahscript also handles this, too, so if we ever do fix it, gotta fix it there too
      }
      if (auto* q = context.get_owning_quest()) {
         if (auto* e = dovah::story_event_definition::lookup(q->event))
            if (auto* m = e->member_by_signature(member))
               return m->name;
      }
      return QCoreApplication::translate("condition event member, unknown ID", "<event member:%1>").arg(member);
   }

   extern QString event_parameter(
      const dovah::loaded_forms::components::conditions::context& context,
      const dovah::loaded_forms::components::conditions::event_parameters& params,
      size_t n,
      const options::form_format& format
   ) {
      if (n == 0) {
         return event_function(params.function);
      } else if (n == 1) {
         return event_member(context, params.member);
      } else if (n == 2) {
         if (!dovah::conditions::event_function_uses_form(params.function))
            return {};
         return form(params.form.get_form_stub(), format);
      }
      return {};
   }
   extern QString event_parameter(
      const dovah::loaded_forms::components::conditions::context& context,
      const dovah::loaded_forms::components::conditions::working_event_parameters& params,
      size_t n,
      const options::form_format& format
   ) {
      if (n == 0) {
         return event_function(params.function);
      } else if (n == 1) {
         return event_member(context, params.member);
      } else if (n == 2) {
         if (!dovah::conditions::event_function_uses_form(params.function))
            return {};
         return form(params.form, format);
      }
      return {};
   }
}
