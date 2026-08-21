#include "./run_on.h"
#include <QCoreApplication>
#include "dovah/data/conditions/run_on_type.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/Quest.h"
#include "dovah/form_stub.h"
#include "./alias.h"
#include "./form.h"
#include "./package_data.h"

#define STRING(t) QCoreApplication::translate("dovah::conditions::run_on_type, with details", t)
#define STRING_BARE(t) QCoreApplication::translate("dovah::conditions::run_on_type", t)

namespace editor_helpers::condition_to_string {
   extern std::pair<QString, bool> run_on(
      const dovah::loaded_forms::components::conditions::context& context,
      dovah::conditions::run_on_type type,
      uint32_t index,
      const dovah::form_stub* stub,
      const options::form_format& format
   ) {
      switch (type) {
         using enum dovah::conditions::run_on_type;
         case combat_target:
            return { STRING("Combat Target"), false };
         case event_data:
            if (index && !stub) {
               if (auto* quest = context.get_owning_quest()) {
                  auto  code = quest->event;
                  auto* def  = dovah::story_event_definition::lookup(code);
                  if (def) {
                     auto* member = def->member_by_wide_signature(index);
                     if (member)
                        return { STRING("Event Data: %1").arg(member->name), false };
                  }
               }
            }
            return { STRING_BARE("Event Data"), false };
         case linked_ref:
            return { STRING_BARE("Linked Ref"), false };
         case package_data:
            return editor_helpers::condition_to_string::package_data(context, index);
         case quest_alias:
            return alias(context, index, true);
         case reference:
            if (stub) {
               if (stub->formID == dovah::hardcoded_form_ids::PlayerRef)
                  return { QCoreApplication::translate("condition run-on ref", "Player", "run_on_type::reference, player-actor"), false };
               if (!stub->editorID.empty()) {
                  return { QString::fromStdString(stub->editorID), true };
               }
               return { form(stub, format), false };
            }
            return { QCoreApplication::translate("condition run-on ref", "NONE", "run_on_type::reference, no reference"), false };
         case subject:
            return { STRING_BARE("Subject"), false };
         case target:
            return { STRING_BARE("Target"), false };
      }
      return { STRING_BARE("???"), false };
   }
}
