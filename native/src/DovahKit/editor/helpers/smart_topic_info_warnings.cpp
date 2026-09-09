#include "./smart_topic_info_warnings.h"
#include <QCoreApplication>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/skills.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/TopicInfo.h"
#include "dovah/utils/conditions/classify_boolean_comparison.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/message_log/core.h"

namespace {
   using boolean_test_classification = dovah::utils::conditions::boolean_test_classification;

   using run_on_type = dovah::loaded_forms::components::conditions::run_on_type;
}

namespace editor_helpers {
   extern void smart_topic_info_warnings(const dovah::loaded_forms::TopicInfo& form) {
      bool checks_speaker_base_actor = false;
      bool checks_speaker_skill      = false;

      // negative = always false
      // positive = always true
      int base_actor_check_constant_result = 0;

      auto _scan_condition_list = [&](const dovah::loaded_forms::components::condition_list& list) {
         for (const auto& cnd : list) {
            const auto boolean_class = dovah::utils::conditions::classify_boolean_comparison(cnd);
            switch (boolean_class) {
               using enum boolean_test_classification;
               case not_a_boolean_check:
                  continue;
               case always_true:
               case always_false:
                  if (cnd.get_function_id() == dovah::conditions::function_id_by_name("GetIsID")) {
                     base_actor_check_constant_result = 1;
                     if (boolean_class == always_false)
                        base_actor_check_constant_result = -1;
                  }
                  break;
            }

            if (cnd.get_run_on_data().type != run_on_type::subject)
               continue;

            switch (cnd.get_function_id()) {
               case dovah::conditions::function_id_by_name("GetIsID"):
                  checks_speaker_base_actor = true;
                  break;
               case dovah::conditions::function_id_by_name("GetActorValue"):
                  {
                     auto& param    = cnd.get_parameter(0);
                     assert(std::holds_alternative<int32_t>(param));
                     auto  av_index = std::get<int32_t>(param);
                     if (av_index >= dovah::first_skill_actor_value_index && av_index < dovah::first_skill_actor_value_index + dovah::skill_count) {
                        checks_speaker_skill = true;
                     }
                  }
                  break;
            }
         }
      };

      _scan_condition_list(form.conditions.locked);
      _scan_condition_list(form.conditions.normal);

      auto& logger = dovahkit::subsystems::message_log::core::get();
      if (base_actor_check_constant_result != 0) {
         QString message;
         if (base_actor_check_constant_result > 0) {
            message = QCoreApplication::translate(
               "frontend INFO load warnings",
               //
               "TopicInfo %1 contains a strange-looking GetIsID check. The comparison used will "
               "cause this condition to always pass, i.e. it may as well not even be here. Is "
               "this intentional?"
            );
         } else if (base_actor_check_constant_result < 0) {
            message = QCoreApplication::translate(
               "frontend INFO load warnings",
               //
               "TopicInfo %1 contains a strange-looking GetIsID check. The comparison used will "
               "cause this condition to always fail, i.e. this info should be impossible for any "
               "actor to say. Is this intentional?"
            );
         }
         message = message.arg(editor_helpers::form_identifiers_to_string(&form.stub));
         logger.addLogItem(ui::types::log_item(
            message,
            ui::types::log_item_type::message,
               // ^---- arbitrary decision to save `warning` for invalid data and not invalid "design."  
               //       not committing to that as, like, a standard to follow, just yet.
               // 
               //       maybe post-launch we can add a log item type called "question" or "worry" for 
               //       cases like this where the data is well-formed but looks like a mistake on the 
               //       part of a content author.
            ui::types::log_item_context::form_load
         ));
      }
      if (checks_speaker_base_actor && checks_speaker_skill) {
         //
         // Some dialogue in the vanilla game is intended to be spoken by specific actors, and to 
         // check the player's skills. However, Bethesda accidentally ran the skill checks on the 
         // Subject (i.e. the actor saying the line) instead.
         // 
         // Example: [INFO:00014142] "You know what I mean. Forges have personalities, right?"
         //  - Subject.GetIsID(Oengul) == 1 &&
         //  - Subject.GetActorValue(Smithing) >= 50
         // 
         // If you're using a GetIsID check, it'd be strange for you to actually need to check the 
         // skills of the actor in question. It's not necessarily wrong; if the actor levels with 
         // the player, for example, then checking their skill level is a strange but usable enough 
         // proxy for checking whether they've leveled up. But even that feels... unusual.
         //
         logger.addLogItem(ui::types::log_item(
            QCoreApplication::translate(
               "frontend INFO load warnings",
               //
               "TopicInfo %1 checks both the identity (GetIsID) and skills (GetActorValue) of the "
               "actor saying the line. Is this intentional, or did you mean to check the skills of "
               "some other party (e.g. the player) instead?"
            ).arg(editor_helpers::form_identifiers_to_string(&form.stub)),
            ui::types::log_item_type::message,
               // ^---- arbitrary decision to save `warning` for invalid data and not invalid "design."  
               //       not committing to that as, like, a standard to follow, just yet.
               // 
               //       maybe post-launch we can add a log item type called "question" or "worry" for 
               //       cases like this where the data is well-formed but looks like a mistake on the 
               //       part of a content author.
            ui::types::log_item_context::form_load
         ));
      }
   }
}