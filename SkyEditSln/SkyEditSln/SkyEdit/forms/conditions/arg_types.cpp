#include "arg_types.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"
#include "../../output.h"
#include "../../helpers/strings.h"

bool ConditionArgType::loadValue(ConditionArgValue& out, TESPluginSubrecord& subrecord) const noexcept {
   switch (this->underlying) {
      case ConditionArgUnderlyingType::formID:
         return subrecord.read(out.formID);
      default:
         return subrecord.read(out.dword);
   }
}
void ConditionArgType::toString(const ConditionArgValue& value, std::string& out) const noexcept {
   if (this->isEnum) {
      for (auto& ev : this->enumValues) {
         if (value.dword == ev.value) {
            out = ev.string;
            return;
         }
      }
      cobb::sprintf(out, "Invalid %d", value.dword);
      return;
   }
   switch (this->underlying) {
      case ConditionArgUnderlyingType::formID:
         {
            FormStub* stub = LoadOrder::get().getForm(value.formID);
            if (stub) {
               char sig[5];
               FMT_SIGNATURE(formTypeFor(stub->formType).signature, sig);
               cobb::sprintf(out, "[%s:%08X]%s", sig, stub->formID, stub->get_editor_id());
            } else {
               cobb::sprintf(out, "[????:%08X]", value.formID);
            }
         }
         return;
      case ConditionArgUnderlyingType::float32:
         cobb::sprintf(out, "%f", value.float32);
         return;
      case ConditionArgUnderlyingType::character:
         out = (unsigned char)value.byte;
         return;
      default:
         if (this->isSigned)
            cobb::sprintf(out, "%i", value.dword);
         else
            cobb::sprintf(out, "%u", value.dword);
         return;
   }
}
bool ConditionArgType::isValidForEnum(const ConditionArgValue& value) const noexcept {
   if (this->isEnum)
      for (auto& ev : this->enumValues)
         if (ev.value == value.dword)
            return true;
   return false;
}
void ConditionArgType::getEnumValues(std::vector<ConditionArgValue>& out) const noexcept {
   if (!this->isEnum) {
      out.clear();
      return;
   }
   auto count = this->enumValues.size();
   out.resize(count);
   for (uint32_t i = 0; i < count; i++)
      out[i].dword = this->enumValues[i].value;
}

namespace ConditionArgTypes {
   ConditionArgType     None            = ConditionArgType("None", ConditionArgUnderlyingType::none);
   ConditionArgFormType ActorBase       = ConditionArgFormType("ActorBase", { FormType::ActorBase });
   ConditionArgType     AdvanceAction   = ConditionArgType("Advance Action", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(0, "Normal Usage"),
      ConditionEnumValue(1, "Power Attack"),
      ConditionEnumValue(2, "Bash"),
      ConditionEnumValue(3, "Lockpick Success"),
      ConditionEnumValue(4, "Lockpick Broken"),
   });
   ConditionArgType     Alignment       = ConditionArgType("Alignment", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(0, "Good"),
      ConditionEnumValue(1, "Neutral"),
      ConditionEnumValue(2, "Evil"),
      ConditionEnumValue(3, "Very Good"),
      ConditionEnumValue(4, "Very Evil"),
   });
   ConditionArgFormType AssociationType = ConditionArgFormType("Association Type", { FormType::AssociationType });
   ConditionArgType     Axis            = ConditionArgType("Axis", ConditionArgUnderlyingType::character, {
      ConditionEnumValue('X', "X"),
      ConditionEnumValue('Y', "Y"),
      ConditionEnumValue('Z', "Z"),
   });
   ConditionArgType     CastingSource   = ConditionArgType("Casting Source", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(0, "Left"),
      ConditionEnumValue(1, "Right"),
      ConditionEnumValue(2, "Voice"),
      ConditionEnumValue(3, "Instant"),
   });
   ConditionArgFormType Class           = ConditionArgFormType("Class", { FormType::Class });
   ConditionArgType     CrimeType       = ConditionArgType("Crime Type", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(-1, "None"),
      ConditionEnumValue( 0, "Steal"),
      ConditionEnumValue( 1, "Pickpocket"),
      ConditionEnumValue( 2, "Trespass"),
      ConditionEnumValue( 3, "Attack"),
      ConditionEnumValue( 4, "Murder"),
      ConditionEnumValue( 5, "Escape Jail"),
      ConditionEnumValue( 6, "Werewolf Transformation"),
   });
   ConditionArgType     CriticalStage   = ConditionArgType("Critical Stage", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(0, "None"),
      ConditionEnumValue(1, "Goo Start"),
      ConditionEnumValue(2, "Goo End"),
      ConditionEnumValue(3, "Disintegrate Start"),
      ConditionEnumValue(4, "Disintegrate End"),
   });
   ConditionArgFormType EncounterZone   = ConditionArgFormType("Encounter Zone", { FormType::EncounterZone });
   ConditionArgFormType Faction         = ConditionArgFormType("Faction", { FormType::Faction });
   ConditionArgType     Float           = ConditionArgType("Float", ConditionArgUnderlyingType::float32);
   ConditionArgFormType FormList        = ConditionArgFormType("Form List", { FormType::FormList });
   ConditionArgFormType Furniture       = ConditionArgFormType("Furniture", { FormType::Furniture });
   ConditionArgFormType Global          = ConditionArgFormType("Global Variable", { FormType::Global });
   ConditionArgFormType Keyword         = ConditionArgFormType("Keyword", { FormType::Keyword });
   ConditionArgFormType Location        = ConditionArgFormType("Location", { FormType::Location });
   ConditionArgFormType MagicEffect     = ConditionArgFormType("Magic Effect", { FormType::MagicEffect });
   ConditionArgFormType OwnerForm       = ConditionArgFormType("Owner", { FormType::ActorBase, FormType::Faction });
   ConditionArgFormType Package         = ConditionArgFormType("Package", { FormType::Package });
   ConditionArgFormType Perk            = ConditionArgFormType("Perk", { FormType::Perk });
   ConditionArgFormType Quest           = ConditionArgFormType("Quest", { FormType::Quest });
   ConditionArgFormType Race            = ConditionArgFormType("Race", { FormType::Race });
   ConditionArgFormType Scene           = ConditionArgFormType("Scene", { FormType::Scene });
   ConditionArgType     Sex             = ConditionArgType("Axis", ConditionArgUnderlyingType::integer32, {
      ConditionEnumValue(0, "Male"),
      ConditionEnumValue(1, "Female"),
   });
   ConditionArgFormType Shout           = ConditionArgFormType("Shout", { FormType::Shout });
   ConditionArgFormType Spell           = ConditionArgFormType("Spell", { FormType::Spell });
   ConditionArgFormType Voicetype       = ConditionArgFormType("Voicetype", { FormType::Voicetype });
   ConditionArgFormType Weather         = ConditionArgFormType("Weather", { FormType::Weather });
   ConditionArgFormType Worldspace      = ConditionArgFormType("Worldspace", { FormType::Worldspace });
}