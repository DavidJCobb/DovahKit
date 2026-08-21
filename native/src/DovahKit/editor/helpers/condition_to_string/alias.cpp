#include "./alias.h"
#include <QCoreApplication>
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/Quest.h"

namespace editor_helpers::condition_to_string {
   extern std::pair<QString, bool> alias(
      const dovah::loaded_forms::components::conditions::context& context,
      uint32_t alias_id,
      bool trim_alias_name
   ) {
      if (alias_id == -1)
         return { QCoreApplication::translate("condition use of alias", "NONE", "no alias"), false };
      if (const auto* quest = context.get_owning_quest()) {
         const auto* alias = quest->lookup_alias_by_id(alias_id);
         if (alias) {
            auto name = QString::fromStdString(alias->name);
            if (trim_alias_name)
               name = name.trimmed();
            if (!name.isEmpty())
               return { name, true };
         }
      }
      return { QCoreApplication::translate("condition use of alias", "Quest Alias #%1", "quest not found").arg(alias_id), false };
   }
}
