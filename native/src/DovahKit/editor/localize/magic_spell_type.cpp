#include "./magic_spell_type.h"
#include <QCoreApplication>

#define ENUMERATION_TYPE dovah::magic_spell_type
#include "./_macros.h"

namespace editor::localize {
   extern QString magic_spell_type(ENUMERATION_TYPE v) {
      using enum ENUMERATION_TYPE;
      switch (v) {
         case spell:
            return STRING("Spell");
         case disease:
            return STRING("Disease");
         case power_greater:
            return STRING("Greater Power");
         case power_lesser:
            return STRING("Lesser Power");
         case ability:
            return STRING("Ability");
         case poison:
            return STRING("Poison");
         case enchantment_normal:
            return STRING("Enchantment");
         case alchemy_item:
            return STRING("Potion");
         case ingredient:
            return STRING("Ingredient");
         case leveled_spell:
            return STRING("Leveled Spell");
         case addiction:
            return STRING("Addiction");
         case power_voice:
            return STRING("Voice Power");
         case enchantment_staves:
            return STRING("Staff Enchantment");
         case scroll:
            return STRING("Scroll");
      }
      return "";
   }
}