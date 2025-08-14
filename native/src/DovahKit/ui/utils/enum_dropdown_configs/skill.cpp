#include "./skill.h"
#include "dovah/data/skills.h"
#include "editor/helpers/skill_name_to_string.h"

namespace ui::enum_dropdown_configs {
   extern void skill(QComboBox* widget, bool allow_none, bool as_av_indices) {
      if (allow_none) {
         widget->addItem(QObject::tr("None", "skill names"), -1);
      }
      for (size_t i = 0; i < dovah::skill_count; ++i) {
         auto skill = (dovah::skill)i;
         auto name  = editor_helpers::skill_name_to_string(skill);

         int data = (int)skill;
         if (as_av_indices) {
            data += dovah::first_skill_actor_value_index;
         }
         widget->addItem(name, data);
      }
   }
}