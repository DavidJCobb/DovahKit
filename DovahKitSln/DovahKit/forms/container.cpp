#include "container.h"
#include "../esp/LoadOrder.h"
#include "../esp/TESPlugin.h"

void ContainerData::load(TESPluginSubrecord& subrecord) {
   if (subrecord.signature() == 'CNTO') {
      auto& entry = this->entries.emplace_back();
      subrecord.unchecked_read(entry.item);
      subrecord.unchecked_read(entry.count);
      return;
   }
   if (subrecord.signature() == 'COED') {
      //
      // There's a formID followed by an integer/formID union whose type depends on the form 
      // type of the formID preceding it. Fortunately, we load all FormStubs before we load 
      // any one form, so we can identify the type of the owner form from here.
      //
      if (!this->entries.size())
         return;
      auto& entry = *this->entries.rbegin();
      if (subrecord.read(entry.owner)) {
         auto ownerStub = LoadOrder::get().getForm(entry.owner);
         if (ownerStub && ownerStub->formType == FormType::ActorBase)
            subrecord.unchecked_read(entry.global);
         else
            subrecord.unchecked_read(entry.factionRank);
         subrecord.unchecked_read(entry.condition);
      }
      return;
   }
   if (subrecord.signature() == 'COCT') {
      uint32_t count;
      if (subrecord.read(count))
         this->entries.reserve(count);
      return;
   }
   #if _DEBUG
      __debugbreak();
   #else
      assert(false && "ContainerData::load should only be called for COCT, CNTO, and COED subrecords!");
   #endif
}
/*static*/ void ContainerData::generateUseInfo(TESPluginSubrecord& subrecord, FormStub* stub) {
   form_id_t formID;
   if (subrecord.signature() == 'CNTO') {
      if (subrecord.read(formID))
         stub->add_outbound_reference(formID);
      // remaining bytes don't matter
      return;
   }
   if (subrecord.signature() == 'COED') {
      //
      // There's a formID followed by an integer/formID union whose type depends on the form 
      // type of the formID preceding it. Fortunately, by the time we're building Use Info, 
      // we've already identified all forms and their types.
      //
      if (subrecord.read(formID)) {
         stub->add_outbound_reference(formID);
         //
         if (formID) {
            auto ownerStub = LoadOrder::get().getForm(formID);
            if (ownerStub && ownerStub->formType == FormType::ActorBase) {
               if (subrecord.read(formID)) // owner GLOB
                  stub->add_outbound_reference(formID);
            }
         }
      }
      return;
   }
   if (subrecord.signature() == 'COCT') {
      return;
   }
   #if _DEBUG
      __debugbreak();
   #else
      assert(false && "ContainerData::generateUseInfo should only be called for COCT, CNTO, and COED subrecords!");
   #endif
}