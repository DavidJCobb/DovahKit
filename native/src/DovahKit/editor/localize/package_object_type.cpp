#include "./package_object_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_object_type(dovah::packages::object_type v) {
      switch (v) {
         case dovah::packages::object_type::activators:
            return QCoreApplication::translate("dovah::packages::object_type", "Activators");
         case dovah::packages::object_type::actors_any:
            return QCoreApplication::translate("dovah::packages::object_type", "Actors");
         case dovah::packages::object_type::actors_characters:
            return QCoreApplication::translate("dovah::packages::object_type", "Actors (Characters)");
         case dovah::packages::object_type::actors_creatures:
            return QCoreApplication::translate("dovah::packages::object_type", "Actors (Creatures)");
         case dovah::packages::object_type::actor_effects_any:
            return QCoreApplication::translate("dovah::packages::object_type", "Actor Effects");
         case dovah::packages::object_type::actor_effects_range_self:
            return QCoreApplication::translate("dovah::packages::object_type", "Actor Effects (Ranged, Self)");
         case dovah::packages::object_type::actor_effects_range_target:
            return QCoreApplication::translate("dovah::packages::object_type", "Actor Effects (Ranged, Target)");
         case dovah::packages::object_type::actor_effects_range_touch:
            return QCoreApplication::translate("dovah::packages::object_type", "Actor Effects (Touch)");
         case dovah::packages::object_type::alchemy:
            return QCoreApplication::translate("dovah::packages::object_type", "Potions");
         case dovah::packages::object_type::all_combat_wearable:
            return QCoreApplication::translate("dovah::packages::object_type", "All Combat-Wearable Items");
         case dovah::packages::object_type::all_wearable:
            return QCoreApplication::translate("dovah::packages::object_type", "All Wearable Items");
         case dovah::packages::object_type::ammo:
            return QCoreApplication::translate("dovah::packages::object_type", "Ammo");
         case dovah::packages::object_type::armor:
            return QCoreApplication::translate("dovah::packages::object_type", "Armor");
         case dovah::packages::object_type::books:
            return QCoreApplication::translate("dovah::packages::object_type", "Books");
         case dovah::packages::object_type::clothing:
            return QCoreApplication::translate("dovah::packages::object_type", "Clothing");
         case dovah::packages::object_type::containers:
            return QCoreApplication::translate("dovah::packages::object_type", "Containers");
         case dovah::packages::object_type::doors:
            return QCoreApplication::translate("dovah::packages::object_type", "Doors");
         case dovah::packages::object_type::food:
            return QCoreApplication::translate("dovah::packages::object_type", "Food");
         case dovah::packages::object_type::ingredients:
            return QCoreApplication::translate("dovah::packages::object_type", "Ingredients");
         case dovah::packages::object_type::keys:
            return QCoreApplication::translate("dovah::packages::object_type", "Keys");
         case dovah::packages::object_type::lights:
            return QCoreApplication::translate("dovah::packages::object_type", "Lights");
         case dovah::packages::object_type::misc:
            return QCoreApplication::translate("dovah::packages::object_type", "Misc");
         case dovah::packages::object_type::none:
            return QCoreApplication::translate("dovah::packages::object_type", "None");
         case dovah::packages::object_type::flora:
            return QCoreApplication::translate("dovah::packages::object_type", "Flora");
         case dovah::packages::object_type::furniture:
            return QCoreApplication::translate("dovah::packages::object_type", "Furniture");
         case dovah::packages::object_type::weapons_any:
            return QCoreApplication::translate("dovah::packages::object_type", "Weapons");
         case dovah::packages::object_type::weapons_ranged:
            return QCoreApplication::translate("dovah::packages::object_type", "Weapons (Ranged)");
         case dovah::packages::object_type::weapons_melee:
            return QCoreApplication::translate("dovah::packages::object_type", "Weapons (Melee)");
         case dovah::packages::object_type::weapons_none:
            return QCoreApplication::translate("dovah::packages::object_type", "Weapons (None)");
      }
      return "";
   }
}