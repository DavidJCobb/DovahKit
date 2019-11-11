#pragma once
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"

struct FormStub;
class TESPluginFile;
class TESPluginHeader;

enum class LoadErrorCode {
   none = 0,
   //
   // active_file_is_dependency
   // The active file cannot be a master to any other file in the requested 
   // load order. If records in the active file are overridden, we will not 
   // be able to recover the active-file versions of those forms, causing 
   // complications when saving.
   //
   active_file_is_dependency = 1,
   //
   // malformed_file
   // We detected some kind of problem when trying to read a file. This 
   // could indicate something like a record that is too small for its 
   // contents.
   //
   malformed_file = 2,
   //
   // bad_form_id_in_esl
   // Files that have been flagged as light plug-ins can only contain a 
   // limited number of forms, which must exist within a specific range of 
   // local form IDs.
   //
   bad_form_id_in_esl = 3,
   //
   // missing_master
   // One of the files in the load order has a master that could not be 
   // loaded.
   //
   missing_master = 4,
   //
   // missing_file
   // One of the files in the load order doesn't exist.
   //
   missing_file = 5,
   //
   // locked_file
   // We were unable to open one of the requested files, or one of their 
   // masters, apparently because the file in question is locked.
   //
   locked_file = 6,
   //
   // out_of_bounds_form_id
   // A file contained a form with an out-of-bounds form ID, e.g. a file 
   // with three masters (whose local forms would therefore use load order 
   // prefix 04) containing a form with a load order prefix above 04. Note 
   // that this error is NOT triggered by a form with a valid ID containing 
   // a field that refers to an invalid form ID; although that is incorrect, 
   // it isn't an error that prevents loading.
   //
   out_of_bounds_form_id = 7,
   //
   // too_many_files
   // The load order has ended up containing too many files. This can happen 
   // if more than 254 files are selected for load, or if there end up being 
   // more than 254 files that need to load after forcing selected files' 
   // unselected masters to load.
   //
   too_many_files = 8,
};

class LoadOrder {
   public:
      static LoadOrder& get() {
         static LoadOrder instance;
         return instance;
      }
      static constexpr uint8_t invalid_load_prefix = 0xFF;
      //
   protected:
      struct _form_map {
         std::mutex   lock;
         map_of_forms forms;
      };
      //
      // Clients can request that we load a set of files. This set of files is 
      // not required to be complete or in the proper order; if we encounter a 
      // file with an unexpected master, then we'll handle that properly. We 
      // do this by constructing a "final" load order based on the queued files. 
      // For example, if you ask us to load just Update.esm, then we'll look at 
      // its file header and find the unexpected master "Skyrim.esm," and we'll 
      // add that to the load order before we add Update.esm. We do this recurs-
      // ively, and we account for ESMs and ESPs being mixed together and we 
      // reorder things as needed.
      //
      // As such:
      //
      //  - The queued files are in (queuedFiles)
      //
      //  - The final load order, based on reading the headers of each file in 
      //    that queue and accounting for unexpected masters, is broken across 
      //    (loadOrderMasters) and (loadOrderPlugins); treat them as a single 
      //    list.
      //
      //  - Once we've actually loaded the files, they'll be in (files).
      //
      std::set<TESPluginHeader*> loadOrderUnderConsideration; // used to detect cyclical dependencies between files
      std::vector<TESPluginHeader*> loadOrderMasters;
      std::vector<TESPluginHeader*> loadOrderPlugins;
      std::vector<TESPluginFile*>   files;
      TESPluginFile* activeFile = nullptr; // TODO
      _form_map formsByType[FormType::Count];
      //
      uint8_t  loadOrderPrefixFor(TESPluginFile*) const noexcept;
      uint32_t localFormIDToGlobalFormID(FormStub* stub) const;
      //
      typedef decltype(loadOrderMasters)::const_iterator _load_order_it;
      bool _loadOrderHasMaster(const std::string& name) const;
      bool _loadOrderHasPlugin(const std::string& name) const;
      void _moveToMasters(const std::string& name) noexcept;
      uint16_t _loadOrderSize() const noexcept {
         return this->loadOrderMasters.size() + this->loadOrderPlugins.size();
      }
      bool _addToLoadOrder(const std::string& name, bool isMasterOfMaster = false);
      //
   public:
      std::string basePath;
      std::vector<std::string> queuedFiles; // files we plan on loading
      std::string queuedActiveFile; // name of the file that is going to be the active file.
      //
      struct {
         LoadErrorCode code = LoadErrorCode::none;
         std::string   file;
         std::string   dependency;
         std::string   parseError;
         uint32_t      formID     = 0;
         uint32_t      fileOffset = 0;
      } lastError;
      //
      void addFile(const std::string& name);
      void removeFile(const std::string& name);
      void setActiveFile(const std::string& name); // TODO
      bool loadQueuedFiles();
      //
      uint8_t indexOf(const std::string& filename) const noexcept;
      //
      FormStub* getForm(uint32_t formID) const;
      FormStub* getForm(formtype_t formType, uint32_t formID) const;
      void forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)>);
      //
      void acceptFormStub(FormStub*) noexcept; // used during load
};