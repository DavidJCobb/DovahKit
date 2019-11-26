#include "LoadOrder.h"
#include <algorithm>
#include "TESPlugin.h"
#include "TESPluginHeader.h"
#include "../helpers/strings.h"
#include "../output.h"

#define BENCHMARK_LOAD_ORDER_USE_INFO_BUILD 1
#if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
   #include <sys/timeb.h>
#endif

const char* _load_error_code_names[] = {
   "no error",
   "active file is a dependency of another file",
   "file is malformed",
   "ESL contains a form ID that is too high",
   "a dependency is missing",
   "the file is missing",
   "the file is locked and cannot be opened",
   "a form had a bad form ID",
   "there are too many files in the load order",
   "the file is part of a cyclical dependency",
   "unknown/unhandled error",
   "filesystem or file I/O error",
   "insufficient memory available for this data",
};
const char* FatalLoadError::code_string() const noexcept {
   if ((uint32_t)this->code < std::extent<decltype(_load_error_code_names)>::value)
      return _load_error_code_names[(uint32_t)this->code];
   return "<no string>";
}

void LoadOrder::logError(std::function<void(FatalLoadError&)> functor) {
   if (this->loadingIsComplete)
      return;
   std::lock_guard<std::mutex> guard(this->lastErrorLock);
   if (!this->lastError.defined())
      functor(this->lastError);
}

uint8_t LoadOrder::loadOrderPrefixFor(const TESPluginFile* file) const noexcept {
   uint8_t size = this->files.size();
   for (uint8_t i = 0; i < size; i++) {
      if (file == this->files[i])
         return i;
   }
   return 0xFF;
}
uint8_t LoadOrder::_guidedLoadOrderPrefixFor(const TESPluginFile* file) const noexcept {
   //
   // Intended for use during loading. If (this->loadingIndex) is non-zero, then it's the 
   // index of the file currently being loaded; we check that index in the load order first 
   // before searching the full load order. (If we're currently loading file 00, then we 
   // start at zero, which is the same as searching the full load order anyway.)
   //
   uint8_t size = this->files.size();
   uint8_t li   = this->loadingIndex;
   if (li && li < size) {
      auto f = this->files[li];
      if (f == file)
         return li;
   }
   for (uint8_t i = 0; i < size; i++) {
      if (file == this->files[i])
         return i;
   }
   return 0xFF;
}
form_id_status LoadOrder::localFormIDToGlobalFormID(FormStub* stub, uint32_t& out) const {
   if ((stub->formID & plugin_form_id_mask) == 0) { // Handle form IDs for hardcoded forms.
      //
      // All forms between xx000001 and xx0007FF, inclusive, are hardcoded forms and the 
      // load order prefix is ignored.
      //
      out = stub->formID & hardcoded_form_id_mask;
      return form_id_status::valid;
   }
   if (stub->formType == FormType::GameSetting) {
      //
      // Skyrim.esm contains a GMST record with incorrect form ID 0123C00E. Accordingly, 
      // since GMST form IDs clearly don't matter, we need to just make sure we store 
      // them consistently and otherwise not validate them in any way.
      //
      out = stub->formID & 0x00FFFFFF;
      return form_id_status::valid;
   }
   auto    file   = stub->file;
   uint8_t local  = file->masters.size();
   uint8_t prefix = stub->formID >> 0x18;
   if (prefix == local) {
      out = stub->formID & 0x00FFFFFF | (this->_guidedLoadOrderPrefixFor(file) << 0x18);
      return form_id_status::valid;
   }
   if (prefix > local) {
      out = 0;
      return form_id_status::out_of_bounds;
   }
   auto&   name = file->masters[prefix].master;
   uint8_t j    = this->indexOf(name);
   if (j == invalid_load_prefix) {
      out = 0;
      return form_id_status::missing_master;
   }
   out = stub->formID & 0x00FFFFFF | (j << 0x18);
   return form_id_status::valid;
}
TESPluginFile* LoadOrder::getFileByName(const char* name) const noexcept {
   auto i = this->indexOf(name);
   if (i == invalid_load_prefix)
      return nullptr;
   return this->files[i];
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
   auto header = new TESPluginHeader;
   std::string path = this->basePath + name;
   if (!header->load(path.c_str())) {
      this->logError([&name](FatalLoadError& error) {
         error.code = LoadErrorCode::malformed_file;
         error.file = name;
         error.parseError = "Failed initial read of the file header. The file ended too soon.";
      });
      delete header;
      return false;
   }
   //
   if (this->loadOrderUnderConsideration.find(header->name) != loadOrderUnderConsideration.end()) {
      this->logError([this, &name](FatalLoadError& error) {
         error.code = LoadErrorCode::cyclical_dependency_between_files;
         error.file = name;
         error.parseError = "Failed initial read of the file header. This file is part of a cyclical dependency.";
         //
         auto& list = this->loadOrderUnderConsideration;
         if (list.size() > 1) {
            auto last = list.rbegin();
            error.dependency = *last;
         }
      });
      delete header;
      return false;
   }
   this->loadOrderUnderConsideration.insert(header->name);
   //
   bool must_be_master = isMasterOfMaster || header->is_master();
   for (auto it = header->masters.begin(); it != header->masters.end(); ++it) {
      if (this->_loadOrderHasMaster(*it))
         continue;
      if (this->_loadOrderHasPlugin(*it)) {
         if (must_be_master)
            //
            // We allow ESPs and ESMs to be mixed together. This, of course, means that we need 
            // to handle the possibility of a list of queued files containing an ESP, followed 
            // by an ESM that has that ESP as a master.
            //
            this->_moveToMasters(*it);
         continue;
      }
      //
      // If we got here, then we have an unexpected master. Recurse on it -- deal with any 
      // unexpected masters of the unexpected master.
      //
      if (!this->_addToLoadOrder(*it, must_be_master)) {
         this->loadOrderUnderConsideration.erase(header->name);
         delete header;
         return false;
      }
   }
   //
   // And now that we know all of (header)'s masters are in the load order, add (header) itself.
   //
   if (must_be_master)
      this->loadOrderMasters.push_back(header);
   else
      this->loadOrderPlugins.push_back(header);
   this->loadOrderUnderConsideration.erase(header->name);
   //
   if (this->_loadOrderSize() > 254) {
      this->logError([&name](FatalLoadError& error) {
         error.code = LoadErrorCode::too_many_files;
         error.file = name;
         error.parseError = "The load order is too long.";
      });
      this->loadOrderUnderConsideration.erase(header->name);
      delete header;
      return false;
   }
   return true;
}
//
class ThreadedUseInfoOutboundBuilder : public TESPluginFileView {
   protected:
      std::vector<FormStub*> queue;
      std::thread thread;
      //
      static void _thread_handler(ThreadedUseInfoOutboundBuilder* instance) {
         auto registration = UseInfoEntryHeap::get().register_thread();
         instance->_execute();
      }
      void _execute() {
         _DEBUGMSG("[ThreadedUseInfoOutboundBuilder] Thread %08X has started processing %d forms.", std::this_thread::get_id(), this->queue.size());
         #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
            struct timeb bench_start;
            struct timeb bench_last;
            struct timeb bench_current;
            ftime(&bench_last);
            bench_start = bench_last;
            uint32_t i = 0;
            std::thread::id threadID = std::this_thread::get_id();
         #endif
         auto& list = this->queue;
         for (auto it = list.begin(); it != list.end(); ++it) {
            (*it)->build_outbound_refs(this);
            #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
               ftime(&bench_current);
               auto diff = (uint32_t)(1000.0 * (bench_current.time - bench_last.time)) + (bench_current.millitm - bench_last.millitm);
               if (diff > 1000) {
                  diff = (uint32_t)(1000.0 * (bench_current.time - bench_start.time)) + (bench_current.millitm - bench_start.millitm);
                  _DEBUGMSG("[ThreadedUseInfoOutboundBuilder] Thread %08X: Time %d ms: processed %d forms.", threadID, diff, i);
               }
               bench_last = bench_current;
               i++;
            #endif
         }
         _DEBUGMSG("[ThreadedUseInfoOutboundBuilder] Thread %08X has finished processing %d forms.", std::this_thread::get_id(), this->queue.size());
      }
   public:
      void addToQueue(FormStub* stub) noexcept {
         this->queue.push_back(stub);
      }
      void start() noexcept {
         this->thread = std::thread(ThreadedUseInfoOutboundBuilder::_thread_handler, this);
      }
      void wait_for() noexcept {
         if (this->is_active())
            this->thread.join();
      }
      //
      inline bool is_active() const noexcept { return this->thread.get_id() != std::thread::id(); }
};

void LoadOrder::_buildUseInfo() noexcept {
   #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
      struct timeb bench_start;
      struct timeb bench_end;
      printf("Building Use Info...\n");
      ftime(&bench_start);
   #endif
   ThreadedUseInfoOutboundBuilder builders[8];
   uint32_t which_thread = 0;
   // Inbound first, since we can multi-thread that
   for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
      FormStub* stub = it->second;
      builders[which_thread].addToQueue(stub);
      if (++which_thread > 7)
         which_thread = 0;
   }
   for (int i = 0; i < std::extent<decltype(builders)>::value; i++)
      builders[i].start();
   for (int i = 0; i < std::extent<decltype(builders)>::value; i++)
      builders[i].wait_for();
   #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
      ftime(&bench_end);
      printf("Time taken for outbound refs: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
      ftime(&bench_start);
   #endif
   // Outbound next; has to be single-threaded
   for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
      it->second->send_inbound_refs();
   }
   #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
      ftime(&bench_end);
      printf("Time taken for inbound refs: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   #endif
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
   this->loadingIsComplete = false;
   if (!this->basePath.empty()) {
      char end = *this->basePath.rbegin();
      if (end != '/' && end != '\\')
         this->basePath += '/';
   }
   //
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
      this->loadingIndex = this->files.size();
      this->files.push_back(file);
      if (!file->load(path.c_str())) {
         auto fn = (*it)->name;
         this->logError([&fn](FatalLoadError& error) {
            error.code       = LoadErrorCode::unknown_error;
            error.file       = fn;
            error.parseError = "Failed to load a master.";
         });
         return false;
      }
      //
      // TODO: Split file loading into these steps:
      //
      // 1. Load the header.
      // 2. Verify that there aren't any unexpected masters (i.e. file wasn't altered 
      //    between constructing the load order and now). If there are, fail.
      // 3. Load the rest of the file.
      //
   }
   for (auto it = this->loadOrderPlugins.begin(); it != this->loadOrderPlugins.end(); ++it) {
      std::string path = this->basePath + (*it)->name;
      auto file = new TESPluginFile;
      this->loadingIndex = this->files.size();
      this->files.push_back(file);
      if (!file->load(path.c_str())) {
         auto fn = (*it)->name;
         this->logError([&fn](FatalLoadError& error) {
            error.code       = LoadErrorCode::unknown_error;
            error.file       = fn;
            error.parseError = "Failed to load a plugin.";
         });
         return false;
      }
      //
      // TODO: Split file loading into these steps:
      //
      // 1. Load the header.
      // 2. Verify that there aren't any unexpected masters (i.e. file wasn't altered 
      //    between constructing the load order and now). If there are, fail.
      // 3. Load the rest of the file.
      //
   }
   this->loadingIsComplete = true;
   this->loadingIndex      = 0;
   //
   this->_buildUseInfo();
   //
   return !this->lastError.defined();
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

bool LoadOrder::hasForm(uint32_t formID) const noexcept {
   if (formID == 0)
      return false;
   auto& list = this->forms.forms;
   auto  it   = list.find(formID);
   return (it != list.end());
}
FormStub* LoadOrder::getForm(uint32_t formID) const noexcept {
   if (formID == 0)
      return nullptr;
   auto& list = this->forms.forms;
   auto  it   = list.find(formID);
   if (it != list.end())
      return it->second;
   return nullptr;
}
FormStub* LoadOrder::getForm(uint8_t formType, uint32_t formID) const noexcept {
   if (formID == 0)
      return nullptr;
   if (formType < std::extent<decltype(this->formsByType)>::value) {
      auto& list = this->formsByType[formType].forms;
      auto  it   = list.find(formID);
      if (it != list.end())
         return it->second;
   }
   return nullptr;
}
FormStub* LoadOrder::getFormOfProbableType(formtype_t formType, uint32_t formID) const noexcept {
   if (formID == 0)
      return nullptr;
   if (formType < std::extent<decltype(this->formsByType)>::value) {
      auto& list = this->formsByType[formType].forms;
      auto  it   = list.find(formID);
      if (it != list.end())
         return it->second;
   }
   return this->getForm(formID);
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
form_id_status LoadOrder::localFormIDToGlobalFormID(const TESPluginFile* file, uint32_t& id) const {
   if ((id & plugin_form_id_mask) == 0) { // hardcoded
      id = id & hardcoded_form_id_mask;
      return form_id_status::valid;
   }
   if (!file) {
      id = 0;
      return form_id_status::missing_master;
   }
   uint8_t local  = file->masters.size();
   uint8_t prefix = id >> 0x18;
   if (prefix == local) {
      id = id & 0x00FFFFFF | (this->_guidedLoadOrderPrefixFor(file) << 0x18);
      return form_id_status::valid;
   }
   if (prefix > local) {
      id = 0;
      return form_id_status::out_of_bounds;
   }
   auto&   name = file->masters[prefix].master;
   uint8_t j    = this->indexOf(name);
   if (j == invalid_load_prefix) {
      id = 0;
      return form_id_status::missing_master;
   }
   id = id & 0x00FFFFFF | (j << 0x18);
   return form_id_status::valid;
}

void LoadOrder::reset() {
   this->loadingIsComplete = false;
   this->loadingIndex      = 0;
   this->lastError.reset();
   this->loadOrderUnderConsideration.clear();
   for (auto it = this->loadOrderMasters.begin(); it != this->loadOrderMasters.end(); ++it)
      delete (*it);
   this->loadOrderMasters.clear();
   for (auto it = this->loadOrderPlugins.begin(); it != this->loadOrderPlugins.end(); ++it)
      delete (*it);
   this->loadOrderPlugins.clear();
   //
   for (auto it = this->files.begin(); it != this->files.end(); ++it)
      delete (*it);
   this->files.clear();
   this->activeFile = nullptr;
   //
   this->queuedFiles.clear();
   this->queuedActiveFile.clear();
   //
   this->forms.forms.clear();
   for (formtype_t ft = 0; ft < std::extent<decltype(this->formsByType)>::value; ft++)
      this->formsByType[ft].forms.clear();
   FormStubHeap::get().force_free_all();
}

form_id_status LoadOrder::acceptFormStub(FormStub* stub) noexcept {
   auto& type = this->formsByType[stub->formType];
   std::lock_guard<std::mutex> guard_for_form_type(type.lock);
   std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
   //
   uint32_t formID;
   auto     result = this->localFormIDToGlobalFormID(stub, formID);
   switch (result) {
      case form_id_status::out_of_bounds:
      case form_id_status::missing_master:
         return result;
   }
   if (formID == 0)
      return form_id_status::null_is_not_allowed;
   //
   // update the map of forms by type:
   //
   FormStub*& target = type.forms[formID];
   if (target) // is this an override?
      delete target; // delete the overridden form stub
   target = stub;
   //
   // update the map of all forms as well:
   //
   this->forms.forms[formID] = stub;
   //
   stub->formID = formID;
   //
   return form_id_status::valid;
}