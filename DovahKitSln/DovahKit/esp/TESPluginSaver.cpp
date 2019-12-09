#include "TESPluginSaver.h"
#include "LoadOrder.h"
#include <algorithm>

void TESPluginRecordSaver::write(void* source, uint32_t size) noexcept {
   this->owner.write(source, size);
   this->header.size += size;
}

TESPluginRecordSaver& TESPluginSubrecordSaver::get_containing_record() const {
   return this->owner.record;
}
void TESPluginSubrecordSaver::write(void* source, uint32_t size) noexcept {
   this->get_containing_record().write(source, size);
   this->header.size += size;
}

/*static*/ bool TESPluginSaver::_sortStubsForSave(const FormStub* a, const FormStub* b) noexcept {
   //
   // xEdit prefers to sort forms by form ID when saving, but the Creation Kit isn't 
   // guaranteed to do so, especially when version control is in use. To minimize 
   // changes to a file when (re)saving it, sort forms by their file offset (if they 
   // have been modified) or their form ID (if they're new).
   //
   if (!b->file) {
      if (a->file)
         return true;
      return a->formID < b->formID;
   }
   if (!a->file)
      return false;
   return a->offset < b->offset;
}
void TESPluginSaver::save() noexcept {
   this->file = fopen("out.tes", "wbx");
   if (!this->file) {
      assert(false && "TODO: HANDLE THIS ERROR");
      return;
   }
   uint32_t recordCount = 0; // TODO
   uint32_t nextFormID  = 0; // TODO
   //
   // TODO: Collate all records to save, so we can fill the above vars for the file header
   //
   auto& record    = this->record;
   auto& subrecord = this->subrecord;
   {  // File header
      this->openRecord('TES4', 0);
      this->openSubrecord('HEDR');
      subrecord.write(0.94F);
      subrecord.write(recordCount);
      subrecord.write(nextFormID);
      subrecord.close();
      this->write_single_field_subrecord('CNAM', this->header.author.c_str());
      this->write_single_field_subrecord('SNAM', this->header.description.c_str());
      assert(false && "Write the code to save masters (MAST/DATA)!");
      assert(false && "Write the code to load and save master overrides (ONAM)!");
      if (this->source) {
         this->write_single_field_subrecord('INTV', (uint32_t)this->source->subINTV);
         this->write_single_field_subrecord('INCC', (uint32_t)this->source->subINCC);
      } else {
         this->write_single_field_subrecord('INTV', (uint32_t)0);
      }
      record.close();
   }
   auto& gsl = groupSequence;
   for (uint32_t i = 0; i < gsl.count; i++) {
      auto signature = gsl[i];
      auto formType  = signatureToFormType(signature);
      if (!formType)
         continue;
      std::vector<FormStub*> forms;
      LoadOrder::get().forEachFormOfType(formType, [&forms](FormStub* stub) {
         forms.push_back(stub);
         return false;
      });
      std::sort(forms.begin(), forms.end(), TESPluginSaver::_sortStubsForSave);

   }
}