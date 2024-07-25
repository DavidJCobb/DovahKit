#include "./skill_name_to_string.h"
#include <QCoreApplication>

namespace editor_helpers {
   extern QString skill_name_to_string(dovah::skill skill) {
      switch (skill) {
         case dovah::skill::alchemy:
            return QCoreApplication::translate("all skill names", "Alchemy");
         case dovah::skill::alteration:
            return QCoreApplication::translate("all skill names", "Alteration");
         case dovah::skill::archery:
            return QCoreApplication::translate("all skill names", "Archery");
         case dovah::skill::block:
            return QCoreApplication::translate("all skill names", "Block");
         case dovah::skill::conjuration:
            return QCoreApplication::translate("all skill names", "Conjuration");
         case dovah::skill::destruction:
            return QCoreApplication::translate("all skill names", "Destruction");
         case dovah::skill::enchanting:
            return QCoreApplication::translate("all skill names", "Enchanting");
         case dovah::skill::heavy_armor:
            return QCoreApplication::translate("all skill names", "Heavy Armor");
         case dovah::skill::illusion:
            return QCoreApplication::translate("all skill names", "Illusion");
         case dovah::skill::light_armor:
            return QCoreApplication::translate("all skill names", "Light Armor");
         case dovah::skill::lockpicking:
            return QCoreApplication::translate("all skill names", "Lockpicking");
         case dovah::skill::one_handed:
            return QCoreApplication::translate("all skill names", "One-Handed");
         case dovah::skill::pickpocket:
            return QCoreApplication::translate("all skill names", "Pickpocket");
         case dovah::skill::restoration:
            return QCoreApplication::translate("all skill names", "Restoration");
         case dovah::skill::smithing:
            return QCoreApplication::translate("all skill names", "Smithing");
         case dovah::skill::sneak:
            return QCoreApplication::translate("all skill names", "Sneak");
         case dovah::skill::speech:
            return QCoreApplication::translate("all skill names", "Speech");
         case dovah::skill::two_handed:
            return QCoreApplication::translate("all skill names", "Two-Handed");
      }
      return "";
   }
}