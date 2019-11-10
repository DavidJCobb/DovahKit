#include "LoadOrder.h"
#include "TESPlugin.h"

uint8_t LoadOrder::loadOrderPrefixFor(TESPluginFile* file) const noexcept {
   uint8_t size = this->files.size();
   for (uint8_t i = 0; i < size; i++) {
      if (file == this->files[i])
         return i;
   }
   return 0xFF;
}
uint32_t LoadOrder::localFormIDToGlobalFormID(FormStub* stub) const {
   if ((stub->formID & plugin_form_id_mask) == 0) { // Handle form IDs for hardcoded forms.
      //
      // All forms between xx000001 and xx0007FF, inclusive, are hardcoded forms and the 
      // load order prefix is ignored.
      //
      assert(stub->formID != 0 && "Null form ID.");
      return stub->formID & hardcoded_form_id_mask;
   }
   if (stub->formType == FormType::GameSetting) {
      //
      // Skyrim.esm contains a GMST record with incorrect form ID 0123C00E. Accordingly, 
      // since GMST form IDs clearly don't matter, we need to just make sure we store 
      // them consistently and otherwise not validate them in any way.
      //
      return stub->formID & 0x00FFFFFF;
   }
   auto    file   = stub->file;
   uint8_t local  = file->masters.size();
   uint8_t prefix = stub->formID >> 0x18;
   if (prefix == local)
      return stub->formID & 0x00FFFFFF | (this->loadOrderPrefixFor(file) << 0x18);
   if (prefix > local) {
      //
      // TODO: Invalid form ID. Fail and abort all loading.
      //
      assert(false && "TODO: Write code to handle out-of-bounds form IDs.");
   }
   auto&   name = file->masters[prefix].master;
   uint8_t j    = this->indexOf(name);
   assert(j != invalid_load_prefix && "We failed to handle a missing master somewhere.");
   return stub->formID & 0x00FFFFFF | (j << 0x18);
}

void LoadOrder::addFile(const std::string& name) {
   auto it = std::find(this->queuedFiles.begin(), this->queuedFiles.end(), name);
   if (it == this->queuedFiles.end())
      this->queuedFiles.push_back(name);
}
void LoadOrder::removeFile(const std::string& name) {
   auto it = std::find(this->queuedFiles.begin(), this->queuedFiles.end(), name);
   if (it != this->queuedFiles.end())
      this->queuedFiles.erase(it);
}
bool LoadOrder::loadQueuedFiles() {
   assert(this->files.size() == 0 && "Must clear loaded files before you can use a new load order!");
   //
   for (auto it = this->queuedFiles.begin(); it != this->queuedFiles.end(); ++it) {
      std::string path = this->basePath + *it;
      auto file = new TESPluginFile;
      this->files.push_back(file);
      if (!file->load(path.c_str()))
         return false;
   }
   return true;
}

uint8_t LoadOrder::indexOf(const std::string& filename) const noexcept {
   auto size = this->files.size();
   for (uint8_t i = 0; i < size; i++) {
      auto  file = this->files[i];
      auto& name = file->getFilename();
      //
      // We can't use std::string::operator== because that compares the 
      // strings' sizes first, as an optimization... which breaks, because 
      // it's possible for one of these strings to contain a trailing null 
      // and for the other string not to. Specifically, a TESPluginFile's 
      // listed masters won't have a trailing null because there isn't one 
      // in the MAST subrecords in the file header.
      //
      if (_stricmp(name.data(), filename.data()) == 0)
         return i;
   }
   return invalid_load_prefix;
}

FormStub* LoadOrder::getForm(uint32_t formID) const {
   for (formtype_t ft = 0; ft < std::extent<decltype(this->formsByType)>::value; ft++) {
      auto& list = this->formsByType[ft].forms;
      try {
         return list.at(formID);
      } catch (std::out_of_range) {}
   }
   return nullptr;
}
FormStub* LoadOrder::getForm(uint8_t formType, uint32_t formID) const {
   if (formType < std::extent<decltype(this->formsByType)>::value) {
      try {
         return this->formsByType[formType].forms.at(formID);
      } catch (std::out_of_range) {}
   }
   return nullptr;
}
void LoadOrder::forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)> functor) {
   if (formType < std::extent<decltype(this->formsByType)>::value) {
      auto& list = this->formsByType[formType].forms;
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (functor(it->second))
            break;
      }
   }
}

void LoadOrder::acceptFormStub(FormStub* stub) noexcept {
   auto& type = this->formsByType[stub->formType];
   std::lock_guard<std::mutex> guard(type.lock);
   //
   uint32_t   formID = this->localFormIDToGlobalFormID(stub);
   FormStub*& target = type.forms[formID];
   if (target) // is this an override?
      delete target;
   target = stub;
}