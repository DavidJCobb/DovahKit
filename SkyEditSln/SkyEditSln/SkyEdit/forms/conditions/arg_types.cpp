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
   void Float::toString(const ConditionArgValue& value, std::string& out) const noexcept {
      cobb::sprintf(out, "%f", value.float32);
   }
   //
   void Sex::toString(const ConditionArgValue& value, std::string& out) const noexcept {
      switch (value.dword) {
         case Sex::male:
            out = "Male";
            return;
         case Sex::female:
            out = "Female";
            return;
      }
      cobb::sprintf(out, "Invalid %d", value.dword);
   }
   bool Sex::isValidForEnum(const ConditionArgValue& value) const noexcept {
      switch (value.dword) {
         case Sex::male:
         case Sex::female:
            return true;
      }
      return false;
   }
   void Sex::getEnumValues(std::vector<ConditionArgValue>& out) const noexcept {
      out.resize(2);
      out[0].dword = Sex::male;
      out[1].dword = Sex::female;
   }
}