#pragma once
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include "../formstub.h"
#include "../formstub_maps.h"
#include "../forms/types.h"

class FormStub;
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
   // prefix 04) containing a form with a load order prefix above 04; or a 
   // file contained a form whose form ID corresponds to a master that, 
   // for unknown reasons, failed to load.
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
   //
   // cyclical_dependency_between_files
   // The load order contains files whose lists of masters form a cyclical 
   // dependency.
   //
   cyclical_dependency_between_files = 9,
   //
   // unknown_error
   // An error was caught somewhere "above" where it actually happened, and 
   // no details are available. If this error code is ever actually seen, 
   // it indicates that I forgot to have an error check report specific error 
   // details.
   //
   unknown_error = 10,
   //
   // filesystem_error
   // Generic codes for filesystem errors e.g. "too many open files."
   //
   filesystem_error = 11,
   //
   // insufficient_memory
   // The file contains a record that is impossible to load or parse due to 
   // its massive size. When this occurs, it may be a sign that something 
   // went wrong during parsing, and that we're misreading unrelated data 
   // as a record length.
   //
   insufficient_memory = 12,
   //
   // active_file_is_master_and_there_are_plugins
   // The active file must be at the end of the load order. However, it is 
   // impossible to ensure this, because the active file is ESM-flagged 
   // and there are non-ESMs in the load order.
   //
   active_file_is_master_and_there_are_plugins = 13,
   //
   // REMEMBER TO KEEP FatalLoadError::code_string SYNCHED WITH THIS ENUM!
   //
};
struct FatalLoadError {
   LoadErrorCode code = LoadErrorCode::none;
   std::string   file;
   std::string   dependency;
   std::string   parseError;
   uint32_t      formID     = 0;
   uint32_t      fileOffset = 0;
   //
   inline bool defined() const noexcept { return this->code != LoadErrorCode::none; }
   void reset() noexcept {
      this->code = LoadErrorCode::none;
      this->file.clear();
      this->dependency.clear();
      this->parseError.clear();
      this->formID     = 0;
      this->fileOffset = 0;
   }
   const char* code_string() const noexcept;
   //
   operator bool() const noexcept { return this->defined(); }
};

enum class form_id_status {
   valid,
   out_of_bounds,
   missing_master,
   null_is_not_allowed,
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
         mutable std::mutex lock;
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
      std::set<std::string> loadOrderUnderConsideration; // used to detect cyclical dependencies between files
      std::vector<TESPluginHeader*> loadOrderMasters;
      std::vector<TESPluginHeader*> loadOrderPlugins;
      std::vector<TESPluginFile*>   files;
      TESPluginFile* activeFile = nullptr; // TODO
      _form_map forms;
      _form_map formsByType[FormType::Count];
      _form_map activeFileForms;
      _form_map activeFileFormsByType[FormType::Count];
      //
      uint8_t loadOrderPrefixFor(const TESPluginFile*) const noexcept;
      uint8_t _guidedLoadOrderPrefixFor(const TESPluginFile*) const noexcept; // a version of (loadOrderPrefixFor) that's faster if called on a TESPluginFile that we're currently loading
      form_id_status localFormIDToGlobalFormID(FormStub* stub, uint32_t& out) const;
      TESPluginFile* getFileByName(const char* name) const noexcept;
      //
      bool _loadOrderHasMaster(const std::string& name) const;
      bool _loadOrderHasPlugin(const std::string& name) const;
      void _moveToMasters(const std::string& name) noexcept;
      uint16_t _loadOrderSize() const noexcept {
         return this->loadOrderMasters.size() + this->loadOrderPlugins.size();
      }
      bool _addToLoadOrder(const std::string& name, bool isMasterOfMaster = false);
      //
      void _buildUseInfo() noexcept;
      //
      FatalLoadError lastError;
      std::mutex lastErrorLock;
      //
      bool    loadingIsComplete = false; // exists so that TESPluginBaseReader::nextSubrecord can call LoadOrder::logError without having to worry about whether it's running during or after the initial load
      uint8_t loadingIndex      = 0;     // which load order index we're loading, or 0 if none; set in (loadQueuedFiles); see (_guidedLoadOrderPrefixFor)
      uint8_t activeFileIndex   = invalid_load_prefix;
      //
   public:
      std::string basePath;
      std::vector<std::string> queuedFiles; // files we plan on loading
      std::string queuedActiveFile; // name of the file that is going to be the active file.
      //
      struct {
         bool allowUnknownRecordSignatures = false;
         bool allowSuspiciousRecordSignatures = false;
      } options;
      //
      void addFile(const std::string& name);
      void removeFile(const std::string& name);
      void setActiveFile(const std::string& name); // TODO
      bool loadQueuedFiles();
      //
      inline bool isLoading() const noexcept { return !this->loadingIsComplete; };
      //
      uint8_t indexOf(const std::string& filename) const noexcept;
      //
      bool hasForm(uint32_t formID) const noexcept;
      FormStub* getForm(uint32_t formID) const noexcept;
      FormStub* getForm(formtype_t formType, uint32_t formID) const noexcept; // use when you KNOW the form's type
      FormStub* getFormOfProbableType(formtype_t formType, uint32_t formID) const noexcept; // searches (formType) first, then the other types
      void forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)>);
      form_id_status localFormIDToGlobalFormID(const TESPluginFile* file, uint32_t& id) const;
      //
      // reset
      // Clears last-error details, wipes the load order, and deletes all FormStubs -- and I do mean 
      // ALL FormStubs -- from memory. Nothing should still have a pointer to a FormStub or a loaded 
      // form when this is called, or you'll get undefined behavior.
      //
      void reset();
      //
      // acceptFormStub
      // Used by TESPluginFile to store a newly-loaded form stub. If the newly-loaded stub originates 
      // from an override record, then the overridden record's stub is deleted and replaced -- we 
      // only retain the last-loaded record for any given form ID, like the game and the CK.
      //
      form_id_status acceptFormStub(FormStub*) noexcept;
      //
      // logError
      // Calls the given lambda to log error details, but only if there isn't already another logged 
      // error. Use like this:
      //
      // LoadOrder::get().logError([this, stub](FatalLoadError& error) {
      //    error.code       = LoadErrorCode::out_of_bounds_form_id;
      //    error.file       = this->name;
      //    error.fileOffset = this->getPos();
      //    error.formID     = stub->formID;
      //    error.parseError = "A form cannot use xx000000 as its form ID.";
      // });
      //
      // Only one error can be kept at a time, but this is only meant for irrecoverable errors that 
      // occur during the load process, so that's generally not a concern. The function uses a 
      // thread lock, so it should be good for use with multi-threaded loading code. The reason it 
      // skips the lambda if there already is an error is so you can write code that logs different 
      // kinds of failures at different points in the load process; e.g.
      //
      // // Logs an error upon seeing identifiably incorrect data in the file header; returns true if 
      // // the whole header is loaded or false if early EOF or file error:
      // bool loadHeader();
      //
      // // Calls loadHeader. If that returns false, logs a catch-all error message assuming we hit 
      // // an early EOF. If loadHeader logged a more specific error message, then our catch-all 
      // // doesn't get logged, and that's how we want it.
      // bool loadWholeFile();
      //
      // Note also that this function does nothing if called while the load process isn't running.
      //
      void logError(std::function<void(FatalLoadError&)>);
      //
      // getError
      // NOT thread-safe; only obtain this to display an error after loading has failed.
      //
      inline const FatalLoadError& getError() const noexcept { return this->lastError; };
};