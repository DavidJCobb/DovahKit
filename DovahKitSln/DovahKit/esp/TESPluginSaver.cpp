#include "TESPluginSaver.h"

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