#include "LoadOrder.h"
#include <algorithm>
#include "TESPlugin.h"
#include "TESPluginHeader.h"
#include "../helpers/strings.h"
#include "../output.h"

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

bool LoadOrder::_loadOrderHasMaster(const std::string& name) const {
   auto& list = this->loadOrderMasters;
   for (auto it = list.begin(); it != list.end(); ++it)
      if (cobb::strieq(name, (*it)->name))
         return true;
   return false;
}
bool LoadOrder::_loadOrderHasPlugin(const std::string& name) const {
   auto& list = this->loadOrderPlugins;
   for (auto it = list.begin(); it != list.end(); ++it)
      if (cobb::strieq(name, (*it)->name))
         return true;
   return false;
}
void LoadOrder::_moveToMasters(const std::string& name) noexcept {
   auto& list = this->loadOrderPlugins;
   auto  it   = std::find_if(list.begin(), list.end(), [&name](TESPluginHeader* file) { return cobb::strieq(name, file->name); });
   if (it == list.end())
      return;
   TESPluginHeader* header = *it;
   list.erase(it);
   //
   auto& masterNames = header->masters;
   for (auto jt = masterNames.begin(); jt != masterNames.end(); ++jt) {
      this->_moveToMasters(*jt);
   }
   this->loadOrderMasters.push_back(header);
}
bool LoadOrder::_addToLoadOrder(const std::string& name, bool isMasterOfMaster) {
   //
   // TODO: Need to handle cyclical references between files
   //
   auto header = new TESPluginHeader;
   std::string path = this->basePath + name;
   if (!header->load(path.c_str())) {
      _DEBUGMSG("ERROR: Failed to open: %s", path.c_str());
      delete header;
      return false;
   }
   if (this->loadOrderUnderConsideration.find(header) != loadOrderUnderConsideration.end()) {
      //
      // ERROR: cyclical reference.
      //
      _DEBUGMSG("ERROR: Detected a cyclical dependency between files.");
      delete header;
      return false;
   }
   this->loadOrderUnderConsideration.insert(header);
   bool must_be_master = isMasterOfMaster || header->is_master();
   for (auto it = header->masters.begin(); it != header->masters.end(); ++it) {
      if (this->_loadOrderHasMaster(*it))
         continue;
      if (this->_loadOrderHasPlugin(*it)) {
         if (must_be_master)
            this->_moveToMasters(*it);
         continue;
      }
      //
      // We have an unexpected master. Recurse on it.
      //
      if (!this->_addToLoadOrder(*it, must_be_master)) {
         this->loadOrderUnderConsideration.erase(header);
         delete header;
         return false;
      }
      if (this->_loadOrderSize() > 254) {
         //
         // ERROR: Load order is too long.
         //
         _DEBUGMSG("ERROR: The effective load order is too long.");
         this->loadOrderUnderConsideration.erase(header);
         delete header;
         return false;
      }
   }
   if (must_be_master)
      this->loadOrderMasters.push_back(header);
   else
      this->loadOrderPlugins.push_back(header);
   this->loadOrderUnderConsideration.erase(header);
   return true;
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
   /*
   for (auto it = this->queuedFiles.begin(); it != this->queuedFiles.end(); ++it) {
      std::string path = this->basePath + *it;
      auto file = new TESPluginFile;
      this->files.push_back(file);
      if (!file->load(path.c_str()))
         return false;
   }
   */
   for (auto it = this->queuedFiles.begin(); it != this->queuedFiles.end(); ++it) {
      if (!this->_addToLoadOrder(*it))
         return false;
   }
   {
      _DEBUGMSG("Final load order:");
      for (auto it = this->loadOrderMasters.begin(); it != this->loadOrderMasters.end(); ++it)
         _DEBUGMSG("[M] %s", (*it)->name.c_str());
      for (auto it = this->loadOrderPlugins.begin(); it != this->loadOrderPlugins.end(); ++it)
         _DEBUGMSG("[P] %s", (*it)->name.c_str());
   }
   for (auto it = this->loadOrderMasters.begin(); it != this->loadOrderMasters.end(); ++it) {
      std::string path = this->basePath + (*it)->name;
      auto file = new TESPluginFile;
      this->files.push_back(file);
      if (!file->load(path.c_str()))
         return false;
   }
   for (auto it = this->loadOrderPlugins.begin(); it != this->loadOrderPlugins.end(); ++it) {
      std::string path = this->basePath + (*it)->name;
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
      if (cobb::strieq(name, filename))
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