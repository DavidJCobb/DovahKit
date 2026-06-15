#include "./file_load_order.h"
#include "helpers/performance.h"
#include "helpers/string/strieq_ascii.h"
#include "helpers/unordered_map.h"
#include "../data/game/hardcoded_form_ids_ignore_record_id_prefix.h"
#include "../data/form_id_constants.h"
#include "../exceptions/enums/form_creation_error_code.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "../game_change_failure_reason.h"
#include "./tes_file_reading/file_loader.h"
#include "./tes_file_reading/file_header.h"
#include "./tes_file_reading/threaded_load_order_use_info_builder.h"
#include "./tes_file_reading/load_order_persistent_ref_reparenter.h"
#include "./tes_file_writing/file_writer.h"
#include "./tes_file_writing/results.h"
#include "../forms/factories/construct.h"
#include "../forms/factories/hardcoded.h"
#include "../forms/Form.h"
#include "../logging.h"
#include "./bsa/bsa_load_order.h"
#include "../utils/form_type_is_cell_child.h"
#include "../utils/get_ini_defined_bsa_list.h"
#include "../utils/get_user_language_name.h"
#include "../localization/localized_string_store.h"
#include <fstream>

#include "../exceptions/invalid_load_order/active_file_is_master_and_there_are_plugins.h"
#include "../exceptions/invalid_load_order/some_files_are_too_new.h"
#include "../exceptions/file_load_failed.h"
#include "../exceptions/file_save_failed.h"
#include "../exceptions/form_creation_failed.h"
#include "../exceptions/form_deletion_failed.h"
#include "../exceptions/form_renumber_failed.h"
#include "../exceptions/game_change_failed.h"
#include "../exceptions/game_setting_renumber_failed.h"
#include "../exceptions/game_setting_value_change_failed.h"
#include "../load_order_interfaces/file_load.h"
#include "../load_order_processes/file_save.h"
#include "../load_order_requests/form_creation_request.h"
#include "../load_order_requests/form_deletion_request.h"
#include "../load_order_requests/form_duplication_request.h"
#include "../load_order_requests/form_renumber_request.h"
#include "../load_order_requests/game_setting_edit_request.h"
#include "../load_order_requests/game_setting_renumber_request.h"
#include "../notices/file_load_errors/form_id_is_invalid.h"
#include "../notices/file_load_errors/form_override_has_type_mismatch.h"
#include "../notices/file_load_warnings/esl_defined_an_interior_cell.h"
#include "../notices/file_load_warnings/esl_defined_interior_cell_is_overridden.h"
#include "../notices/file_load_warnings/form_initial_record_is_partial.h"
#include "../notices/file_load_warnings/form_override_has_armo_arma_mismatch.h"
#include "../notices/file_load_warnings/game_setting_has_multiple_records_in_a_file.h"
#include "../notices/file_load_warnings/game_setting_name_is_unrecognized.h"
#include "../notices/file_load_warnings/game_setting_overrides_a_real_form.h"
#include "../notices/file_load_warnings/game_setting_record_has_bad_form_id.h"
#include "../notices/file_load_warnings/game_setting_record_has_no_name.h"
#include "../notices/file_load_warnings/partial_info_override_has_different_parent.h"
#include "../notices/file_load_warnings/singleton_form_is_redundantly_defined.h"
#include "../notices/file_load_warnings/the_game_doesnt_load_new_actor_value_infos.h"
#include "../notices/form_load_warnings/form_reference_type_mismatch.h"
#include "../notices/base_form_load_warning.h"

namespace {
   //
   // Several parts of the  backend will double-check that  all forms relevant to a 
   // task can be edited in certain ways. If all form types are known to have fully-
   // implemented loaded-form classes, then these checks are unnecessary and can be 
   // skipped. Setting this bool to (true) will skip these tasks.
   //
   constexpr bool ALL_FORM_TYPES_ARE_IMPLEMENTED_YES_IM_SURE = false;
}

namespace dovah {
   game_setting_type loaded_game_setting::get_type() const noexcept {
      return get_game_setting_type_from_name(this->name.c_str());
   }
   void loaded_game_setting::set_value(const game_setting_value& v) noexcept {
      switch (this->get_type()) {
         case game_setting_type::boolean:
            this->value.b = v.b;
            break;
         case game_setting_type::float32:
            this->value.f = v.f;
            break;
         case game_setting_type::integer:
            this->value.i = v.i;
            break;
         case game_setting_type::string:
            this->value.s = v.s;
            break;
      }
   }

   file_load_order::~file_load_order() {
      this->normalizer.delete_contents();
      this->queued_load.files.clear();
      this->queued_load.active_file.clear();
      //
      for (auto& f : this->forms.forms) {
         auto stub = f.second;
         if (stub)
            delete stub;
      }
      this->forms.forms.clear();
      for (auto& m : this->forms_by_type)
         m.forms.clear();
      this->active_file_forms.forms.clear();
      for (auto& m : this->active_file_forms_by_type)
         m.forms.clear();
      //
      for (auto* f : this->files)
         delete f;
      this->files.clear();
      this->active_file = nullptr;
      if (auto* file = this->hardcoded_forms_file) {
         delete file;
         this->hardcoded_forms_file = nullptr;
      }
      if (auto* file = this->none_stubs_file) {
         delete file;
         this->none_stubs_file = nullptr;
      }
      //
      if (this->archives) {
         delete this->archives;
         this->archives = nullptr;
      }
   }

   file_load_order::_save_load_lock_guard::_save_load_lock_guard(file_load_order& o, file_load_order::save_load_type desired) : owner(o) {
      auto& lock  = owner.save_load_state.type;
      auto  prior = save_load_type::none;
      if (!lock.compare_exchange_strong(prior, desired))
         return;
      this->success = true;
   }
   file_load_order::_save_load_lock_guard::~_save_load_lock_guard() {
      if (this->success)
         owner.save_load_state.type = save_load_type::none;
   }

   void file_load_order::adopt_archive_list(bsa_load_order& list) noexcept {
      this->archives = &list;
   }

   #pragma region File loading
   void file_load_order::_make_hardcoded_forms() {
      load_order_interfaces::file_load dummy(*this);
      this->hardcoded_forms_file = new tes_file_reading::file_loader(dummy);
      this->hardcoded_forms_file->header.details |= tes_file_reading::file_loader::detail_flag::is_hardcoded_dummy;
      //
      add_hardcoded_forms_to_load_order(*this);
   }
   void file_load_order::_accept_hardcoded_form(form_stub* stub) noexcept {
      auto& type = this->forms_by_type[stub->form_type];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      //
      stub->_add_file(*this->hardcoded_forms_file, 0);
      stub->flags |= form_stub::flag::is_hardcoded;
      //
      bare_form_id_t formID = stub->formID;
      type.forms[formID] = stub;
      this->forms.forms[formID] = stub;
   }
   void file_load_order::_build_none_stubs() {
      load_order_interfaces::file_load dummy(*this);
      this->none_stubs_file = new tes_file_reading::file_loader(dummy);
      this->none_stubs_file->header.details |= tes_file_reading::file_loader::detail_flag::is_none_stub_dummy;
      //
      std::vector<bare_form_id_t> formIDs;
      std::vector<form_stub*> users;
      for (auto& pair : this->forms.forms) {
         auto* stub = pair.second;
         if (!stub)
            continue;
         bool found_any = false;
         for (auto& pair : stub->outbound) {
            if (pair.second.other) // if there's a stub pointer, then this isn't a dangling form-to-form reference
               continue;
            formIDs.push_back(pair.first);
            found_any = true;
         }
         if (found_any)
            users.push_back(stub);
      }
      if (formIDs.empty())
         return;
      for (auto id : formIDs) {
         auto* stub = new form_stub;
         stub->formID    = id;
         stub->form_type = form_type::none;
         stub->_add_file(*this->none_stubs_file, 0);
         //
         this->forms_by_type[form_type::none].forms[id] = stub;
         this->forms.forms[id] = stub;
      }
      //
      // Simply creating the none-stubs isn't enough, though: the use info in the 
      // referencing forms needs to be updated to point to the new none-stubs.
      //
      for (auto* user : users) {
         for (auto& pair : user->outbound) {
            if (pair.second.other)
               continue;
            pair.second.other = this->get_form(form_type::none, pair.first);
         }
      }
   }
   void file_load_order::_reparent_persistent_references() {
      //
      // All of a worldspace's persistent references are encoded as children of the persistent 
      // cell. However, this is cumbersome for frontends to work with, and what's more, it's not 
      // necessary within the backend: when saving, we already handle the case of persistent refs 
      // existing in non-persistent cells.
      //
      tes_file_reading::load_order_persistent_ref_reparenter::get().execute(*this);
   }
   void file_load_order::_build_use_info() {
      //
      // We generate Use Info using two passes. First, we divide all forms across multiple 
      // threads; each thread generates outbound Use Info for the forms. Then, a single 
      // thread scans the outbound Use Info and uses that to generate inbound Use Info.
      //
      // If we want to generate outbound Use Info for a given form, then we only need to 
      // update that form. However, if we want to generate *inbound* Use Info *from* a 
      // given form, we must update each form it refers to. As such, we can't multi-thread 
      // the generation of inbound Use Info unless we put a lock on each individual form.
      // 
      // Note that parent form relationships, tracked during load, exist as outbound use 
      // info.
      //
      auto& builders = this->use_info_build_threads;
      {
         auto guard     = std::lock_guard(this->use_info_build_threads_lock);
         auto count_per = this->forms.forms.size() / builders.size();
         for (auto& entry : builders) {
            entry = new tes_file_reading::threaded_load_order_use_info_builder;
            entry->reserve(count_per + 1);
         }
      }
      int which_thread = 0;
      // Inbound first, since we can multi-thread that
      for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
         form_stub* stub = it->second;
         builders[which_thread]->add_to_queue(stub);
         if (++which_thread >= this->use_info_build_threads.size())
            which_thread = 0;
      }
      for (auto* thread : builders)
         thread->start();
      for (auto* thread : builders)
         thread->wait_for();
      {
         const size_t no_offset = builders.size();

         size_t earliest_offset = no_offset;
         for (size_t i = 0; i < builders.size(); ++i) {
            auto* thread = builders[i];
            if (!thread->exception.captured)
               continue;

            if (earliest_offset == no_offset) {
               earliest_offset = i;
            } else {
               if (thread->exception.thrown_at_file_offset < builders[earliest_offset]->exception.thrown_at_file_offset)
                  earliest_offset = i;
            }
         }
         if (earliest_offset != no_offset) {
            auto captured = std::move(builders[earliest_offset]->exception.captured);

            // per below
            auto guard = std::lock_guard(this->use_info_build_threads_lock);
            for (auto*& entry : builders) {
               delete entry;
               entry = nullptr;
            }

            std::rethrow_exception(captured);
         }
      }
      {
         auto guard = std::lock_guard(this->use_info_build_threads_lock);
         for (auto*& entry : builders) {
            delete entry;
            entry = nullptr;
         }
         this->save_load_state.flags |= save_load_flag::use_info_outbound_complete;
      }
      //
      // These next steps have to be single-threaded. First, we need to build any 
      // none-stubs that are needed; then, we need to use outbound use info to 
      // build inbound use info. None-stubs need to be built first so that the 
      // references to them are properly made bidirectional.
      //
      this->_build_none_stubs();
      for (auto& pair : this->forms.forms) {
         auto* stub = pair.second;
         if (!stub || stub->form_type == form_type::none) // none-type forms (which are usually, but not always, none-stubs) should never have outbound references, so we can skip them
            continue;
         stub->send_inbound_refs({});
      }
   }

   void file_load_order::queue_file(const std::string& name) {
      auto& list = this->queued_load.files;
      auto it = std::find(list.begin(), list.end(), name);
      if (it == list.end())
         list.push_back(name);
   }
   void file_load_order::unqueue_file(const std::string& name) {
      auto& list = this->queued_load.files;
      auto it = std::find(list.begin(), list.end(), name);
      if (it != list.end())
         list.erase(it);
   }
   void file_load_order::queue_active_file(const std::string& name) {
      this->queued_load.active_file = name;
   }
   void file_load_order::load_queued_files() {
      _save_load_lock_guard save_load_lock_guard(*this, save_load_type::is_loading);
      if (!save_load_lock_guard) {
         throw exceptions::file_load_failed(exceptions::file_load_failed::error_code::save_or_load_already_in_progress);
      }

      this->save_load_state.flags = save_load_flag::none;
      struct _ {
         ~_() {
            owner.save_load_state.flags = save_load_flag::none;
         }

         file_load_order& owner;
      } _ugly_raii_hack(*this); // ensure fields are reset if an exception is thrown
      
      if (!this->base_path.empty()) {
         char end = *this->base_path.rbegin();
         if (end != '/' && end != '\\')
            this->base_path += '/';
      }
      //
      this->normalizer.base_path   = this->base_path;
      this->normalizer.active_file = this->queued_load.active_file;
      this->normalizer.target_game = this->current_game;
      for (auto it = this->queued_load.files.begin(); it != this->queued_load.files.end(); ++it) {
         this->normalizer.add(*it);
      }
      {  // Check all files to ensure they aren't too new.
         const auto max_version = game_feature_support::max_file_version(this->current_game);

         std::vector<std::string> too_new_files;
         //
         for (auto* header : this->normalizer.masters)
            if (header->file_version > max_version)
               too_new_files.push_back(header->name);
         for (auto* header : this->normalizer.plugins)
            if (header->file_version > max_version)
               too_new_files.push_back(header->name);

         if (!too_new_files.empty()) {
            auto ex = exceptions::invalid_load_order_exceptions::some_files_are_too_new(std::move(too_new_files));
            throw ex;
         }
      }
      if (!this->queued_load.active_file.empty()) {  // Force the active file to the end of the load order
         //
         // The active file must be at the end of the load order, and cannot be the master of any 
         // other file. Both of these limitations stem from the fact that we only keep the last-
         // loaded version of any record in memory. If the active file is the master to another 
         // file, then that file can override forms defined in the active file; if we save those 
         // overrides into the active file, then we can end up with dangling references between 
         // forms (if the overrides referred to other forms in the overriding file) or with a 
         // cyclical reference (if we try to make the overriding file a master of the active file).
         //
         // If the active file A and another file B both override the same form in some common 
         // master C, and if B loads after A, then we'll have a problem: because we only retain 
         // the last override to load, when we encounter B's override, we will delete A's override 
         // from memory, leaving a dangling pointer in the active file form maps. We could solve 
         // this by testing every form against those maps, but that's not performant.
         //
         auto& name = this->queued_load.active_file;
         bool isMaster = this->normalizer.has_master(name);
         if (isMaster && !this->normalizer.plugins.empty()) {
            auto ex = exceptions::invalid_load_order_exceptions::active_file_is_master_and_there_are_plugins();
            throw ex;
         }
         auto* list = &this->normalizer.plugins;
         if (isMaster)
            list = &this->normalizer.masters;
         //
         auto it = std::find_if(list->begin(), list->end(), [&name](loaded_header* file) { return cobb::strieq_ascii(name, file->name); });
         assert(it != list->end() && "How is it not in the list?!");
         loaded_header* header = *it;
         list->erase(it);
         list->push_back(header);
      }
      if (this->archives) {
         //
         // If we have a (bsa_load_order), then prepend the Skyrim.ini-specified BSAs followed by the 
         // BSAs for each loaded file. We add to the start of the BSA list in order to allow a frontend 
         // to load additional BSAs that override the INI and file ones.
         //
         this->archives->set_base_path(this->base_path);
         //
         uint32_t count = 0;
         //
         auto list = utils::get_ini_defined_bsa_list(this->current_game);
         for (auto& path : list)
            this->archives->insert_archive(count++, path);
         //
         for (auto* header : this->normalizer.masters) {
            if (header->name.empty())
               continue;
            std::filesystem::path name = header->name;
            name.replace_extension(".bsa");
            {
               std::filesystem::path path = this->archives->get_base_path() / name;
               std::ifstream stream(path);
               if (!stream.good())
                  continue;
            }
            this->archives->insert_archive(count++, name);
         }
         for (auto* header : this->normalizer.plugins) {
            if (header->name.empty())
               continue;
            std::filesystem::path name = header->name;
            name.replace_extension(".bsa");
            {
               std::filesystem::path path = this->archives->get_base_path() / name;
               std::ifstream stream(path);
               if (!stream.good())
                  continue;
            }
            this->archives->insert_archive(count++, name);
         }
         //
         this->archives->load_archives();
      }
      this->_make_hardcoded_forms();
      {  // Ensure enough space to store all forms without reallocating
         uint32_t total = 0;
         for (auto* header : this->normalizer.masters)
            total += header->record_and_group_count;
         for (auto* header : this->normalizer.plugins)
            total += header->record_and_group_count;
         this->forms.forms.reserve((size_t)(total * 1.1) + 0x800);
      }
      {
         dovah::logging::print_line("Final load order:");
         for (auto* header : this->normalizer.masters)
            dovah::logging::print_line("[M] %s", header->name.c_str());
         for (auto* header : this->normalizer.plugins)
            dovah::logging::print_line("[P] %s", header->name.c_str());
      }
      //
      auto file_load_interface = load_order_interfaces::file_load(*this);
      {
         using list_t = decltype(this->normalizer.masters);
         //
         auto lambda = [this, &file_load_interface](list_t& list) {
            for (auto* header : list) {
               std::string path = this->base_path + header->name;
               auto* file = new tes_file_reading::file_loader(file_load_interface);
               this->save_load_state.loading_index = this->files.size();
               this->files.push_back(file);
               if (!this->queued_load.active_file.empty() && cobb::strieq_ascii(this->queued_load.active_file, header->name)) {
                  this->active_file = file;
               }
               try {
                  file->load(path.c_str());
               } catch (...) {
                  if (this->archives)
                     this->archives->abort_archive_load();
                  throw;
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
         };
         lambda(this->normalizer.masters);
         lambda(this->normalizer.plugins);
      }
      //
      this->save_load_state.flags |= save_load_flag::loading_is_complete;
      this->save_load_state.loading_index = 0;
      //
      this->_build_use_info(); // may throw
      this->_reparent_persistent_references();
      this->save_load_state.flags |= save_load_flag::use_info_build_is_complete;
      //
      if (!this->is_light_plugin_support_enabled()) {
         //
         // If the user chooses to save a Skyrim Classic file as a Skyrim Special file, then we will 
         // at that time (attempt to) enable light plug-in support. If any already-loaded files were 
         // wrongfully flagged, then doing so will cause tons of errors: we'll retroactively honor 
         // their "light" flag, but without having moved their forms to the 0xFE range. The solution? 
         // Clear the flag for all files during a Classic-only load.
         //
         for (auto* file : this->files)
            file->header.flags &= ~tes_file_flag::light;
      }
      //
      if (!this->active_file && this->files.size() < (this->is_light_plugin_support_enabled() ? 253 : 254)) {
         auto* file = new tes_file_reading::file_loader(file_load_interface);
         this->active_file = file;
         this->files.push_back(file);
      }
      if (this->active_file) {
         auto first_free = this->find_first_free_form_id_in_active_file();
         if (first_free)
            this->active_file->header.nextFormID = first_free;
      }
      //
      if (this->archives) {
         this->archives->wait_for_archive_load_to_finish();
         //
         auto language = utils::get_user_language_name();
         for (auto* file : this->files) {
            if (!file || (file->header.flags & tes_file_flag::localized_string_table) == 0)
               continue;
            file->localization_data = new localized_string_store(*this->archives, file->get_filename());
            file->localization_data->set_default_language(language);
            file->localization_data->open_language_files(language);
         }
      }
      for (auto& pair : this->game_settings.by_name) {
         //
         // String GMSTs hold localizable strings, not constant strings. This means that after localization 
         // files become available, we need to swipe the appropriate string values for any GMSTs that need 
         // them.
         //
         if (get_game_setting_type_from_name(pair.first.c_str()) != game_setting_type::string)
            continue;
         auto& list = pair.second;
         for (auto& entry : list) {
            auto& field = entry.value.s;
            if (field.localized == localization_language::none)
               continue;
            if (auto* base = entry.source_file) {
               if (auto* store = base->localization_data) {
                  field.value     = store->lookup(field.type, field.index);
                  field.localized = store->get_default_language_enum();
               }
            }
         }
      }
   }
   //
   bool file_load_order::is_loading() const noexcept {
      return this->save_load_state.type == save_load_type::is_loading;
   };

   file_load_order::form_id_status file_load_order::accept_form_stub(form_stub*& stub) {
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);

      uint32_t formID;
      auto     result = this->local_formID_to_global_formID(stub, formID);
      if (result != form_id_status::valid || formID == 0) {
         using error_type = dovah::notices::file_load_errors::form_id_is_invalid;

         auto problem = error_type::problem_code::unknown;
         switch (result) {
            case file_load_order::form_id_status::missing_master:
               problem = error_type::problem_code::missing_master;
               break;
            case file_load_order::form_id_status::out_of_bounds:
               problem = error_type::problem_code::out_of_bounds;
               break;
            default:
               if (formID == 0)
                  problem = error_type::problem_code::zero_is_not_allowed;
               break;
         }

         const auto* source_file = stub->get_file_at_index(-1);

         auto error = std::make_unique<error_type>(problem);
         if (source_file) {
            error->filename = source_file->get_filename();
            error->file_offset = ((const tes_file_reading::basic_reader*)source_file)->get_position(); // TODO: HACK HACK HACK
         }
         error->form = {
            .local_id = stub->formID,
            .type = stub->form_type,
         };

         auto ex = dovah::exceptions::file_load_failed();
         ex.details.file_load_error = std::move(error);
         throw ex;
      }

      auto* new_parent = stub->get_parent_form();

      form_stub* existing_form = nullptr;
      {
         auto it = this->forms.forms.find(formID);
         if (it != this->forms.forms.end()) {
            existing_form = it->second;
         }
      }

      if (existing_form) { // is this an override?
         auto type_a = existing_form->form_type;
         auto type_b = stub->form_type;
         if (type_a != type_b) {
            //
            // We're loading an override, but its form type doesn't match that of the overridden form. 
            // This is a hard error in almost all cases, but there is one exception: Skyrim allows 
            // ARMA and ARMO records to override each other. This is a legacy behavior originating 
            // from Fallout 3's engine. We should warn on all mismatched overrides, but skip any 
            // mismatched overrides that aren't ARMA/ARMO.
            //
            bool is_armo_arma = (type_a == form_type::armor || type_b == form_type::armor) && (type_a == form_type::armor_addon || type_b == form_type::armor_addon);
            //
            auto* file_a = existing_form->get_file_at_index(0);
            auto* file_b = stub->get_file_at_index(-1);

            if (is_armo_arma) {
               notices::file_load_warnings::form_override_has_armo_arma_mismatch notice(*existing_form);
               if (file_b)
                  notice.source_file = file_b->get_filename();
               if (file_a)
                  notice.overridden_form.source_file = file_a->get_filename();
               notice.overriding_form.source_file = notice.source_file;
               notice.overriding_form.form_ids = {
                  .local  = stub->formID,
                  .global = formID,
               };
               this->_log_warning(notice);
            } else {
               auto error = std::make_unique<dovah::notices::file_load_errors::form_override_has_type_mismatch>();
               if (file_b) {
                  error->filename    = file_b->get_filename();
                  error->file_offset = ((tes_file_reading::basic_reader*)file_b)->get_position();
               }
               error->form_id = formID;
               error->overriding_form = {
                  .local_id = stub->formID,
                  .type     = stub->form_type,
               };
               error->overridden_form = {
                  .type = existing_form->form_type,
               };
               if (file_a)
                  error->overridden_form.source_file = file_a->get_filename();
               if (file_b)
                  error->overriding_form.source_file = file_b->get_filename();

               auto ex = dovah::exceptions::file_load_failed();
               ex.details.file_load_error = std::move(error);
               throw ex;
            }
         }
         //
         // Delete the new stub, and overwrite the pointer (in this function and in our caller(s)) with 
         // the existing stub.
         //
         if (stub->form_type == form_type::topic_info) {
            //
            // If the stub is a topic info and it's being re-parented by an override, then we need to 
            // update the form stub addenda for its old parent.
            //
            auto* old_parent = existing_form->get_parent_form();
            if (old_parent && old_parent != new_parent) {
               if (auto* addenda = old_parent->addenda) {
                  bool is_active_file = stub->get_file_at_index(-1) == this->active_file;
                  addenda->ordered_children._remove_child_on_load({}, is_active_file, *existing_form);
               }
               //
               if (stub->test_record_flags(tes_file_record_header::flag::partial)) {
                  //
                  // A partial INFO override that re-parents the info? Oh, the game won't handle this 
                  // sanely at all. Refer to <topic infos' placement in topics' info lists.txt> in our 
                  // internal documentation for details on the chaos.
                  //
                  notices::file_load_warnings::partial_info_override_has_different_parent notice;
                  {
                     assert(!stub->has_multiple_source_files()); // the input stub should've been read by ONE file
                     notice.source_file = stub->file.pointer->get_filename();
                  }
                  if (auto* file = stub->get_file_at_index(-1)) {
                     notice.source_file = file->get_filename();
                  }
                  notice.record.local_id  = stub->formID;
                  notice.record.global_id = formID;
                  notice.parent_of_overridden = old_parent;
                  notice.parent_of_overriding = new_parent;
                  if (auto* data = existing_form->get_source_file_info(0)) {
                     if (auto* pointer = data->pointer) {
                        notice.source_file_for_overridden = pointer->get_filename();
                     }
                  }
                  this->_log_warning(notice);
               }
            }
         }
         if (stub->form_type == form_type::cell) {
            if (stub->test_record_flags(tes_file_record_header::flag::persistent)) {
               auto* old_parent = existing_form->get_parent_form();
               if (old_parent && old_parent != new_parent) { // Re-parenting cells isn't supported and isn't a sane operation, but no harm in covering it here
                  auto* addenda = old_parent->addenda;
                  if (addenda && addenda->persistent_cell == existing_form)
                     addenda->persistent_cell = nullptr;
               }
            }
            if (!existing_form->get_parent_form()) {
               auto* file_a = existing_form->get_file_at_index(0);
               auto* file_b = stub->get_file_at_index(-1);
               if (file_a && file_a->is_light()) {
                  notices::file_load_warnings::esl_defined_interior_cell_is_overridden notice(*existing_form);
                  if (file_b)
                     notice.source_file = file_b->get_filename();
                  notice.overridden_form.source_file = file_a->get_filename();
                  notice.overriding_form.source_file = notice.source_file;
                  notice.overriding_form.form_ids = {
                     .local  = stub->formID,
                     .global = formID,
                  };
                  this->_log_warning(notice);
               }
            }
         }
         assert(!stub->has_multiple_source_files()); // the input stub should've been read by ONE file
         existing_form->_add_file(*stub->file.pointer, stub->file.offset, stub->file.flags);
         existing_form->_set_parent_form_one_way({}, new_parent);
         delete stub;
         stub = existing_form;
      } else {
         //
         // This is not an override.
         //
         if (stub->form_type == form_type::actor_value_info) {
            auto notice = notices::file_load_warnings::the_game_doesnt_load_new_actor_value_infos(*stub);
            notice.subject_form_id = formID;
            if (auto* file = stub->get_file_at_index(-1)) {
               notice.source_file = file->get_filename();
            }
            this->_log_warning(notice);
         }
         if (stub->source_file_count() == 1) {
            if (stub->test_record_flags(tes_file_record_header::flag::partial)) {
               bool injected = stub->is_injected();

               notices::file_load_warnings::form_initial_record_is_partial notice;
               if (auto* file = stub->get_file_at_index(-1)) {
                  notice.source_file = file->get_filename();
               }
               notice.record_is_injected = injected;
               notice.record = {
                  .local_id  = stub->formID,
                  .global_id = formID,
                  .form_type = stub->form_type,
               };
               this->_log_warning(notice);

               if (injected) {
                  //
                  // Skyrim will skip a partial record if it is injected and not an override. We'll 
                  // do the same. It's tempting to retain the form or to create a none-stub, but 
                  // either approach would incorrectly prevent subsequent files from injecting over 
                  // the same form ID.
                  //
                  stub->formID = formID;
                  return form_id_status::injected_partial;
               }
               //
               // Strip the flag for a non-injected non-override, as the game would.
               //
               stub->file.flags &= ~tes_file_record_header::flag::partial;
            }
            if (stub->form_type == form_type::cell && !stub->get_parent_form()) {
               if (auto* file = stub->get_file_at_index(-1); file && file->is_light()) {
                  notices::file_load_warnings::esl_defined_an_interior_cell notice;
                  notice.source_file = file->get_filename();
                  notice.form_ids    = {
                     .local  = stub->formID,
                     .global = formID,
                  };
                  this->_log_warning(notice);
               }
            }
         }
         //
         // Insert the stub into the form maps:
         //
         stub->formID = formID;
         //
         auto& type = this->forms_by_type[stub->form_type];
         // DO NOT lock the all-forms map; we already locked it at the start of the function!!
         std::lock_guard<std::mutex> guard_for_form_type(type.lock);
         this->forms.forms[formID] = stub;
         type.forms[formID] = stub;
      }
      //
      // Quick note: This function is guaranteed never to delete the input stub if an error occurs. In one 
      // of the branches above, we may have deleted the input stub and switched it out for the existing stub 
      // which it overrides, so from this point forward we can't delete the stub.
      // 
      if (stub->form_type == form_type::cell) {
         //
         // Manage the persistent cell for this cell's parent world:
         //
         if (stub->test_record_flags(tes_file_record_header::flag::persistent)) {
            //
            // Ensure that worldspaces are aware of their persistent cells.
            //
            if (new_parent && new_parent->form_type == form_type::worldspace) {
               auto& addenda = new_parent->get_or_create_addenda();
               if (!addenda.persistent_cell) // Only the first persistent-flagged cell loaded by a worldspace will be THE persistent cell.
                  addenda.persistent_cell = stub;
            }
         }
      } else if (stub->form_type == form_type::land) {
         //
         // Manage the canonical landscape for this landscape's parent cell:
         //
         if (new_parent && new_parent->form_type == form_type::cell) {
            new_parent->get_or_create_addenda().canonical_landscape = stub;
         }
      }
      //
      // Place the stub inside of the active file form maps, if appropriate.
      //
      if (this->active_file) {
         if (stub->file_list_includes(this->active_file)) {
            this->active_file_forms.forms[formID] = stub;
            auto& at = this->active_file_forms_by_type[stub->form_type];
            at.forms[formID] = stub;
         }
      }
      //
      // Special-case behaviors.
      //
      if (form_type_info::lookup(stub->form_type).flags & form_type_info::flag::is_singleton) {
         //
         // There are a limited number of form types in Bethesda RPGs that are handled specially: the 
         // game will only ever create one instance of the form in question, and all subsequent records 
         // of that type will then load data for the preexisting instance, even if their form IDs differ. 
         // 
         // We need to replicate this behavior, while also still being aware that each given form ID was 
         // occupied by a definition. In other words, we need to have it so that multiple form stubs can 
         // contribute to "the same" form. The easiest way to manage that is this: when inserting a DOBJ 
         // form stub, grab the last inserted DOBJ form stub and copy its source file information into 
         // the new form stub. (Why only the last inserted DOBJ stub? Because it, in turn, will have the 
         // information from its own predecessor, and so on.)
         //
         auto* prior = this->get_canonical_instance_of_singleton_form(stub->form_type);
         if (prior && prior != stub) {
            std::vector<form_stub::file_data> stub_files;
            if (prior) {
               uint16_t count = prior->source_file_count();
               for (uint16_t i = 0; i < count; ++i) {
                  auto* data = prior->get_source_file_info(i);
                  if (!data || !data->pointer)
                     continue;
                  stub_files.push_back(*data);
               }
               //
               // Also, let's log a load warning if this singleton form is redundantly defined within the 
               // current file.
               //
               auto* pf = prior->get_source_file_info();
               auto* sf = stub->get_source_file_info();
               if (pf && sf && pf->pointer == sf->pointer) {
                  notices::file_load_warnings::singleton_form_is_redundantly_defined notice;
                  notice.source_file = sf->pointer->get_filename();
                  notice.previous_form_id = prior->formID;
                  notice.record = {
                     .local_id  = stub->formID,
                     .global_id = formID,
                     .form_type = stub->form_type,
                  };
                  this->_log_warning(notice);
               }
            }
            //
            // And of course, the new stub already knows its file information, so let's grab that, too.
            //
            uint16_t count = stub->source_file_count();
            for (uint16_t i = 0; i < count; ++i) {
               auto* data = stub->get_source_file_info(i);
               if (!data || !data->pointer)
                  continue;
               stub_files.push_back(*data);
            }
            //
            stub->_set_source_file_list(stub_files);
         }
      }
      //
      return form_id_status::valid;
   }
   void file_load_order::accept_game_setting(const loaded_file* file, const loaded_game_setting& working, bare_form_id_t formID) noexcept {
      bare_form_id_t localID = formID;
      //
      auto result = this->local_formID_to_global_formID(file, formID);
      if (result != form_id_status::valid)
         formID = 0;
      //
      if (!formID) {
         notices::file_load_warnings::game_setting_record_has_bad_form_id notice;
         notice.source_file    = file->get_filename();
         notice.form_ids.local = localID;
         if (result == form_id_status::valid)
            notice.form_ids.global = formID;
         notice.setting_name = working.name;
         this->_log_warning(notice);
         if (working.name.empty()) {
            return;
         }
      }
      if (working.name.empty()) {
         notices::file_load_warnings::game_setting_record_has_no_name notice;
         notice.source_file    = file->get_filename();
         notice.form_ids.local = localID;
         if (result == form_id_status::valid)
            notice.form_ids.global = formID;
         this->_log_warning(notice);
         return;
      }
      //
      if (formID) {
         //
         // Do not allow GMST to override form IDs of other types; discard the GMST.
         //
         std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
         //
         form_stub*& prior = this->forms.forms[formID];
         if (prior) {
            if (prior->form_type == form_type::setting) {
               prior->_add_file(*const_cast<loaded_file*>(file), 0);
            } else {
               auto notice = notices::file_load_warnings::game_setting_overrides_a_real_form(working.name, *prior);
               notice.source_file = file->get_filename();
               if (auto* prior_file = prior->get_file_at_index(0)) {
                  notice.overridden_file = prior_file->get_filename();
               }
               this->_log_warning(notice);
               return;
            }
         }
         if (!prior) {
            //
            // Create a form stub, so that this form ID is reserved and can't be used by other forms. Technically, 
            // TESV.exe would allow a form to reuse the GMST form ID if it loaded after the GMST, but given that 
            // we multi-thread form loading, I extremely don't want to try and account for that sort of edge case.
            //
            prior = new form_stub;
            prior->_add_file(*const_cast<loaded_file*>(file), 0);
            prior->formID   = formID;
            prior->form_type = form_type::setting;
         }
         this->forms_by_type[form_type::setting].forms[formID] = prior;
         //
         if (file == this->active_file) {
            this->active_file_forms.forms[formID] = prior;
            this->active_file_forms_by_type[form_type::setting].forms[formID] = prior;
         }
      }
      //
      std::string lowercase;
      lowercase.reserve(working.name.size());
      for (auto c : working.name)
         lowercase += tolower(c);
      //
      std::lock_guard guard(this->game_settings.lock);
      auto& list  = this->game_settings.by_name[lowercase];
      if (!list.empty()) {
         auto& last = list.back();
         if (last.formID != formID && last.source_file == file) {
            bare_form_id_t priorID = last.formID;
            //
            // This is a redundant game setting definition: the game setting was already defined in this file, 
            // but with a different form ID. Let's log a warning before we do anything else.
            //
            notices::file_load_warnings::game_setting_has_multiple_records_in_a_file notice;
            //
            notice.source_file = file->get_filename();
            //
            notice.setting_name           = working.name;
            notice.last_seen_form_id      = priorID;
            notice.current_form_ids.local = localID;
            if (result == form_id_status::valid)
               notice.current_form_ids.global = formID;
            //
            this->_log_warning(notice);
            //
            // What we need to do next depends on what file this came from. If it was the active file, then we 
            // need to delete the old form stub (if no other settings are using it). Why? Well, we don't want 
            // to retain or re-save redundant GMST records in the active file, because that complicates the 
            // process of editing settings' values and the process of saving settings into a file. At the same 
            // time, however, we need to make sure that we handle redundant GMST records in dependencies 
            // properly. We want to support record injection to the fullest extent possible, and if redundant 
            // game settings cause a single setting to take up multiple form IDs within a given file, then we 
            // need to make sure that the user doesn't inject a record onto either of those form IDs -- which 
            // means that we need a stub for each of them.
            //
            // Accordingly, we'll delete the old stub. That will give us consistent behavior with the Creation 
            // Kit: a game setting will have the load seen form ID.
            //
            if (file == this->active_file) {
               if (this->_count_game_settings_with_form_id(priorID) == 1) { // Only delete the form stub if no other settings are using it.
                  std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
                  if (cobb::unordered_map_contains(this->forms.forms, priorID)) {
                     auto* old_stub = this->forms.forms[priorID];
                     delete old_stub;
                     this->forms.forms.erase(priorID);
                     this->forms_by_type[form_type::setting].forms.erase(priorID);
                     this->active_file_forms.forms.erase(priorID);
                     this->active_file_forms_by_type[form_type::setting].forms.erase(priorID);
                  }
               }
               //
               // We're only retaining one definition for this game setting, so remove the prior definition.
               //
               list.resize(list.size() - 1);
            }
         }
      }
      auto& entry = list.emplace_back(working);
      entry.source_file = file;
      entry.formID      = formID;
      if (!entry.definition) {
         notices::file_load_warnings::game_setting_name_is_unrecognized notice;
         notice.source_file    = file->get_filename();
         notice.form_ids.local = localID;
         if (result == form_id_status::valid)
            notice.form_ids.global = formID;
         notice.setting_name = working.name;
         this->_log_warning(notice);
      }
   }

   void file_load_order::_log_warning(const notices::base_warning& notice) {
      if (this->on_warning)
         (this->on_warning)(notice);
   }
   #pragma endregion

   bool file_load_order::_abandon_form_id_reservation(bare_form_id_t id) {
      auto  guard = std::lock_guard(this->form_creation_request_info.lock);
      auto& list  = this->form_creation_request_info.reserved_formIDs;
      //
      auto it = std::find(list.begin(), list.end(), id);
      if (it != list.end()) {
         list.erase(it);
         return true;
      }
      return false;
   }

   bool file_load_order::_can_destroy_none_stub(form_stub& stub) {
      if constexpr (!ALL_FORM_TYPES_ARE_IMPLEMENTED_YES_IM_SURE) {
         //
         // Double-check that we *can* destroy all references to the stub, first.
         //
         for (auto& pair : stub.inbound) {
            auto& data = pair.second;
            if (!data.other || !data.other->load())
               return false;
         }
      }
      return true;
   }
   void file_load_order::_destroy_none_stub(form_stub& stub) {
      assert(stub.is_none_stub());
      assert(this->_can_destroy_none_stub(stub));

      stub.sever_all_outbound_references(); // sever the stub's references to other forms.
      //
      std::vector<form_stub*> pending;
      //
      for (auto& pair : stub.inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         auto  form = other->load();
         pending.push_back(other); // gather forms to process later. we don't want to sever refs now, as that will change use info and potentially invalidate iterators during the loop
         other->set_edited(true);
      }
      for (auto* user : pending) {
         auto form = user->load();
         form->sever_outbound_references_to(stub);
      }
      //
      if (this->on_form_loss)
         (this->on_form_loss)(stub);
      //
      bare_form_id_t formID = stub.formID;
      this->forms.forms.erase(formID);
      this->forms_by_type[stub.form_type].forms.erase(formID);
      this->active_file_forms.forms.erase(formID);
      this->active_file_forms_by_type[stub.form_type].forms.erase(formID);
      //
      delete &stub;
   }

   #pragma region Form renumbering
   void file_load_order::_renumber_form(form_stub& stub, bare_form_id_t new_id, bool update_users) {
      //
      // This function is intended for internal use only. It carries out the core tasks needed to change 
      // a form stub's form ID. This is something you might do for two reasons:
      //
      //   a) As an editing operation, to renumber an individual form's form ID. Such an operation is 
      //      only valid when carried out for forms that originate from the active file.
      //
      //   b) As in-memory bookkeeping, when a file's position in the load order has changed after all 
      //      form stubs have been generated. Generally speaking, this would only occur immediately 
      //      after a successful save operation, and only if the active file has just gained or lost 
      //      the Skyrim Special-exclusive "light plug-in" flag.
      //
      // In the former case, you'd want to pass (true) for (update_users). In the latter case, you'd 
      // want to pass (false).
      //
      if (update_users) {
         //
         // When a form is being renumbered as an actual editing operation (as opposed to in-memory 
         // bookkeeping, of the kind that would occur after saving a file with a changed ESL flag), 
         // we must ensure that all of the form's users remain able to refer to the form. In practice, 
         // we need to load the users and then flag them as edited, and we need to do this before 
         // renumbering the target form. Why? Because if they aren't already loaded, then they will 
         // refer to the target form by its old ID. We need to load them so that they have a reference 
         // to the target form, and then we flag them as edited so that they don't unload (and lose 
         // that reference) until they are saved (at which time they will be updated to use the target's 
         // new form ID).
         //
         // We don't actually need to do anything with the loaded forms; we just need to ensure that 
         // they *are* loaded, so that user forms have pointers to the used forms rather than the old 
         // form ID in the file data.
         //
         for (auto& pair : stub.inbound) {
            auto& entry = pair.second;
            auto  form  = entry.other->load();
            entry.other->set_edited(true);
         }
      }
      bare_form_id_t old_id = stub.formID;
      //
      // Update use info to refer to the new form ID.
      //
      for (auto& pair : stub.outbound) {
         auto& entry = pair.second;
         auto& other = *entry.other;
         //
         auto node = other.inbound.extract(old_id);
         if (node.empty())
            continue;
         node.key() = new_id;
         other.inbound.insert(std::move(node));
      }
      for (auto& pair : stub.inbound) {
         auto& entry = pair.second;
         auto& other = *entry.other;
         //
         auto node = other.outbound.extract(old_id);
         if (node.empty())
            continue;
         node.key() = new_id;
         other.outbound.insert(std::move(node));
      }
      //
      // Update the stub itself to refer to the new form ID.
      //
      stub.formID = new_id;
      //
      // Now move the form stub within the load order's maps.
      //
      auto _extract = [this, old_id, new_id](_form_map& map) {
         auto guard = std::lock_guard(map.lock);
         auto node  = map.forms.extract(old_id);
         if (node.empty())
            return;
         node.key() = new_id;
         map.forms.insert(std::move(node));
      };
      auto form_type = stub.form_type;
      _extract(this->forms);
      _extract(this->forms_by_type[form_type]);
      _extract(this->active_file_forms);
      _extract(this->active_file_forms_by_type[form_type]);
   }
   void file_load_order::_renumber_game_setting(loaded_game_setting& entry, bare_form_id_t new_id) {
      using exception  = exceptions::game_setting_renumber_failed;
      using error_code = exception::error_code;

      if (entry.formID == new_id)
         return;
      if (entry.source_file != this->active_file) {
         #if _DEBUG
            __debugbreak(); // Why are we attempting to renumber a game setting definition that didn't come from the active file?
         #endif
         throw exception(error_code::setting_is_not_in_active_file, entry.name);
      }
      //
      // To renumber a game setting, we need to create or renumber an existing form stub.
      //
      bare_form_id_t old_id = entry.formID;
      //
      auto       form_guard             = std::lock_guard(this->forms.lock);
      uint32_t   stub_count_for_this_id = this->_count_game_settings_with_form_id(old_id);
      form_stub* stub = nullptr;
      if (stub_count_for_this_id == 1) {
         stub = this->forms.forms[old_id];
         if (stub) {
            if (!stub->inbound.empty()) {
               //
               // There are forms that (incorrectly) refer to this GMST's form ID. We need to 
               // sever those references.
               //
               if (!ALL_FORM_TYPES_ARE_IMPLEMENTED_YES_IM_SURE) {
                  //
                  // If DovahKit doesn't yet support all form types, then we're gonna need to 
                  // double-check that the inbound references actually can be severed.
                  //
                  for (auto& pair : stub->inbound) {
                     auto& data = pair.second;
                     if (!data.other || !data.other->load()) {
                        auto ex = exception(error_code::cannot_sever_references_to_setting, entry.name);
                        ex.details.occupying_stub = data.other;
                        throw ex;
                     }
                  }
               }
               //
               // Sever those references!
               //
               for (auto& pair : stub->inbound) {
                  auto& data = pair.second;
                  auto  form = data.other->load();
                  form->sever_outbound_references_to(*stub);
               }
            }
            //
            stub->formID = new_id;
            //
            // Now move the form stub within the load order's maps.
            //
            auto _extract = [this, stub, old_id, new_id](_form_map& map) {
               auto node = map.forms.extract(old_id);
               if (node.empty()) {
                  //
                  // There is no existing node. This can happen if, for example, we are changing 
                  // the form ID of a game setting that was not previously defined in the active 
                  // file.
                  //
                  map.forms[new_id] = stub;
                  return;
               }
               node.key() = new_id;
               map.forms.insert(std::move(node));
            };
            _extract(this->forms);
            _extract(this->forms_by_type[form_type::setting]);
            _extract(this->active_file_forms);
            _extract(this->active_file_forms_by_type[form_type::setting]);
            //
            if (this->on_form_renumber)
               (this->on_form_renumber)(*stub, old_id, new_id);
         }
      }
      if (!stub) {
         //
         // Multiple game setting definitions use the existing stub, or there is no existing stub. 
         // Create a new stub.
         //
         stub = new form_stub;
         stub->formID   = new_id;
         stub->form_type = form_type::setting;
         this->forms.forms[new_id] = stub;
         this->forms_by_type[form_type::setting].forms[new_id] = stub;
         this->active_file_forms.forms[new_id] = stub;
         this->active_file_forms_by_type[form_type::setting].forms[new_id] = stub;
         //
         if (this->on_form_create)
            (this->on_form_create)(stub);
      }
      entry.formID = new_id;
   }
   #pragma endregion

   uint32_t file_load_order::_count_game_settings_with_form_id(bare_form_id_t id) const noexcept {
      uint32_t stub_count_for_this_id = 0;
      if (cobb::unordered_map_contains(this->forms.forms, id)) {
         //
         // A form stub exists for this form ID, which means that the form ID is being used by 
         // at least one game setting. Count the number of loaded game settings that are using 
         // the form ID.
         //
         // For each game setting, we maintain a list of all data supplied by all loaded files. 
         // However, we only care about the last loaded file that defines a setting; that's the 
         // only one that will have a form stub.
         //
         for (auto& pair : this->game_settings.by_name) {
            auto& list = pair.second;
            if (list.empty())
               continue;
            auto& entry = list.back();
            if (entry.formID == id)
               ++stub_count_for_this_id;
         }
      }
      return stub_count_for_this_id;
   }

   #pragma region Changing the current game
   std::optional<game_change_failure_reason> file_load_order::_can_change_current_game(game g, bool because_we_are_changing_whether_the_active_file_is_light) const noexcept {
      if (this->current_game == g)
         return {};
      //
      bool prior_light = game_supports_light_plugins(this->current_game);
      bool after_light = game_supports_light_plugins(g);
      if (prior_light != after_light) {
         if (after_light) {
            //
            // See if we can enable light plug-in support.
            //
            auto size = this->files.size();
            assert(size <= 0xFF); // how do we have more than 255 non-light plug-ins in a load order for a game with no ESL support?
            if (size == 0xFF) {
               //
               // The load order contains enough loaded files to overflow into slot 0xFE.
               //
               if (!because_we_are_changing_whether_the_active_file_is_light)
                  return game_change_failure_reason::load_order_would_overflow_into_lights;
               if (this->active_file && this->files.back() != this->active_file)
                  return game_change_failure_reason::load_order_would_overflow_into_lights;
            }
         } else {
            //
            // See if we can disable light plug-in support. You can't disable light plug-in support if the 
            // load order contains light plug-ins.
            //
            if (!because_we_are_changing_whether_the_active_file_is_light) {
               if (this->active_file && this->active_file->is_light())
                  return game_change_failure_reason::load_order_contains_light_files;
            }
            for (auto* file : this->files) {
               if (file == this->active_file)
                  continue;
               if (file->is_light())
                  return game_change_failure_reason::load_order_contains_light_files;
            }
         }
      }
      return {};
   }
   void file_load_order::_change_current_game(game g, bool because_we_are_changing_whether_the_active_file_is_light) {
      auto code = this->_can_change_current_game(g, because_we_are_changing_whether_the_active_file_is_light);
      if (code.has_value())
         throw exceptions::game_change_failed(code.value(), this->current_game, g);
      this->current_game = g;
   }
   std::optional<game_change_failure_reason> file_load_order::can_change_current_game(game g) const noexcept {
      return this->_can_change_current_game(g, false);
   }
   void file_load_order::change_current_game(game g) {
      return this->_change_current_game(g, false);
   }
   #pragma endregion

   bool file_load_order::is_light_plugin_support_enabled() const noexcept {
      return game_supports_light_plugins(this->current_game);
   }

   float file_load_order::assess_load_progress() const noexcept {
      constexpr float use_info_proportion = 0.2F;
      //
      bool l = this->save_load_state.flags & save_load_flag::loading_is_complete;
      bool u = this->save_load_state.flags & save_load_flag::use_info_build_is_complete;
      if (l) {
         if (u)
            return 1.0F;
         float progress = 0.0F;
         //
         {
            auto guard = std::lock_guard(this->use_info_build_threads_lock);
            for (auto* thread : this->use_info_build_threads)
               if (thread)
                  progress += thread->assess_load_progress();
         }
         if (this->save_load_state.flags & save_load_flag::use_info_outbound_complete)
            return NAN;
         progress /= this->use_info_build_threads.size();
         //
         progress *= use_info_proportion;
         progress += (1.0F - use_info_proportion);
         return progress;
      }
      int count = this->files.size();
      if (this->active_file && this->active_file->get_filename().empty())
         //
         // If we added an implicit active file to the load order, don't count it.
         //
         --count;
      if (count <= 0)
         return 0.0F;
      //
      float per_file = 1.0F / count;
      float progress = per_file * this->save_load_state.loading_index;
      auto* current  = this->files[this->save_load_state.loading_index];
      if (!current)
         return progress;
      auto this_file = current->assess_load_progress();
      if (!isnan(this_file))
         progress += per_file * this_file;
      //
      progress *= (1.0F - use_info_proportion);
      return progress;
   }

   bool file_load_order::is_form_loading_blocked(const form_stub* stub) const noexcept {
      if (this->save_load_state.type != save_load_type::none)
         return true;
      return false;
   }

   #pragma region Finding files
   file_prefix file_load_order::file_prefix_for(const std::filesystem::path& filename) const noexcept {
      uint8_t     heavy = 0xFF;   // start at -1
      uint16_t    light = 0xFFFF; // start at -1
      file_prefix prefix;
      for (auto* f : this->files) {
         bool is_light = this->is_light_plugin_support_enabled() && (f->header.flags & tes_file_flag::light);
         bool is_equal = f->get_filename() == filename;
         if (is_light) {
            ++light;
            if (is_equal)
               prefix = file_prefix::make_light(light);
         } else {
            ++heavy;
            if (is_equal)
               prefix = file_prefix::make_heavy(heavy);
         }
         if (is_equal) {
            if (!game_feature_support::hardcoded_form_ids_ignore_record_id_prefix(this->current_game, f->header.file_version))
               prefix.set_can_co_opt_hardcoded_range(true);
            break;
         }
      }
      return prefix;
   }
   file_prefix file_load_order::file_prefix_for(const loaded_file& file) const noexcept {
      uint8_t     heavy = 0xFF;   // start at -1
      uint16_t    light = 0xFFFF; // start at -1
      file_prefix prefix;
      for (auto* f : this->files) {
         bool is_light = this->is_light_plugin_support_enabled() && (f->header.flags & tes_file_flag::light);
         bool is_equal = f == &file;
         if (is_light) {
            ++light;
            if (is_equal)
               prefix = file_prefix::make_light(light);
         } else {
            ++heavy;
            if (is_equal)
               prefix = file_prefix::make_heavy(heavy);
         }
         if (is_equal) {
            if (!game_feature_support::hardcoded_form_ids_ignore_record_id_prefix(this->current_game, f->header.file_version))
               prefix.set_can_co_opt_hardcoded_range(true);
            break;
         }
      }
      return prefix;
   }
   file_prefix file_load_order::expected_active_file_prefix_post_save(std::optional<bool> pretend_is_or_isnt_light) const noexcept {
      if (!this->active_file)
         return {};

      bool can_co_opt_hardcoded_id_space = false;
      if (!game_feature_support::hardcoded_form_ids_always_ignore_record_id_prefix(this->current_game)) {
         //
         // Even in a game where hardcoded form IDs aren't prefix-agnostic, the active file 
         // can only *use* that ID space if it has at least one master. If the active file 
         // has no masters, then its own record ID prefix is 0x00, and it still has to avoid 
         // the range [00000001, 000007FF].
         //
         if (this->files.size() > 1) {
            can_co_opt_hardcoded_id_space = true;
         }
      }
      file_prefix prefix;
      {
         uint8_t  heavy = 0xFF;   // start at -1
         uint16_t light = 0xFFFF; // start at -1
         for (auto* f : this->files) {
            bool is_light = this->is_light_plugin_support_enabled() && (f->header.flags & tes_file_flag::light);
            bool is_equal = f == this->active_file;
            if (is_equal && pretend_is_or_isnt_light.has_value()) {
               is_light = pretend_is_or_isnt_light.value();
            }
            if (is_light) {
               ++light;
               prefix = file_prefix::make_light(light);
            } else {
               ++heavy;
               prefix = file_prefix::make_heavy(heavy);
            }
            if (is_equal)
               break;
         }
      }
      prefix.set_can_co_opt_hardcoded_range(can_co_opt_hardcoded_id_space);
      return prefix;
   }
   //
   int file_load_order::index_of_file(const loaded_file& f) const noexcept {
      size_t size = this->files.size();
      for (size_t i = 0; i < size; ++i)
         if (this->files[i] == &f)
            return i;
      return -1;
   }
   int file_load_order::index_of_prefix(file_prefix prefix) const noexcept {
      if (prefix.is_undefined())
         return -1;
      size_t size = this->files.size();
      if (prefix.is_light()) {
         uint16_t target = prefix.light_prefix();
         uint16_t count  = -1;
         for (size_t i = 0; i < size; ++i) {
            auto* file = this->files[i];
            if (file->header.flags & tes_file_flag::light) {
               if (++count == target)
                  return i;
            }
         }
         return -1;
      }
      uint8_t target = prefix.load_prefix();
      uint8_t count  = -1;
      for (size_t i = 0; i < size; ++i) {
         auto* file = this->files[i];
         if (!(file->header.flags & tes_file_flag::light)) {
            if (++count == target)
               return i;
         }
      }
      return -1;
   }
   //
   const file_load_order::loaded_file* file_load_order::get_file_by_prefix(file_prefix p) const noexcept {
      auto i = this->index_of_prefix(p);
      if (i < 0)
         return nullptr;
      return this->get_file_by_index(i);
   }
   const file_load_order::loaded_file* file_load_order::get_file_by_index(int i) const noexcept {
      auto size = this->files.size();
      if (i < 0) {
         i += size;
         if (i < 0)
            return nullptr;
      } else if (i >= size)
         return nullptr;
      return this->files[i];
   }
   //
   bool file_load_order::file_is_active(const loaded_file& file) const noexcept {
      return &file == this->active_file;
   }
   bool file_load_order::has_file(const std::filesystem::path& filename) const noexcept {
      for (auto* f : this->files)
         if (f->get_filename() == filename)
            return true;
      return false;
   }
   bool file_load_order::has_non_active_file(const std::filesystem::path& filename) const noexcept {
      if (auto* f = this->active_file)
         if (f->get_filename() == filename)
            return false;
      return this->has_file(filename);
   }
   #pragma endregion

   std::vector<const file_load_order::loaded_file*> file_load_order::get_loaded_files() const noexcept {
      std::vector<const file_load_order::loaded_file*> result;
      result.reserve(this->files.size());
      for (auto* f : this->files)
         result.push_back(f);
      return result;
   }

   uint32_t file_load_order::count_forms_of_type(form_type ft) const noexcept {
      if (decltype(this->forms_by_type)::supports_form_type(ft)) {
         auto& list = this->forms_by_type[ft].forms;
         return list.size();
      }
      return 0;
   }
   bool file_load_order::has_form(bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return false;
      return this->get_form(formID) != nullptr;
   }
   form_stub* file_load_order::get_canonical_instance_of_singleton_form(form_type ft) const noexcept {
      if (!(form_type_info::lookup(ft).flags & form_type_info::flag::is_singleton))
         return nullptr;
      uint16_t   length  = 0;
      form_stub* longest = nullptr;
      auto&      map     = this->forms_by_type[ft].forms;
      for (auto pair : map) {
         auto* stub = pair.second;
         if (!stub)
            continue;
         auto count = stub->source_file_count();
         if (count > length) {
            length  = count;
            longest = stub;
         }
      }
      return longest;
   }
   form_stub* file_load_order::get_canonical_instance_of_singleton_form(form_type ft, bool create_if_missing) noexcept {
      const auto* self = this; // needed to disambiguate between the const and non-const overload
      auto* stub = self->get_canonical_instance_of_singleton_form(ft);
      if (!stub && create_if_missing)
         return this->create_form_of_type(ft);
      return stub;
   }
   form_stub* file_load_order::get_form(bare_form_id_t formID, bool ignore_none_stubs) const noexcept {
      if (formID == 0)
         return nullptr;
      auto& list = this->forms.forms;
      auto  it   = list.find(formID);
      if (it != list.end()) {
         auto* stub = it->second;
         if (ignore_none_stubs && stub && stub->is_none_stub())
            stub = nullptr;
         return stub;
      }
      return nullptr;
   }
   form_stub* file_load_order::get_form(form_type formType, bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return nullptr;
      if (decltype(this->forms_by_type)::supports_form_type(formType)) {
         auto& list = this->forms_by_type[formType].forms;
         auto  it   = list.find(formID);
         if (it != list.end())
            return it->second;
      }
      return nullptr;
   }
   form_stub* file_load_order::get_form_of_probable_type(form_type formType, bare_form_id_t formID, bool ignore_none_stubs) const noexcept {
      if (formID == 0)
         return nullptr;
      if (decltype(this->forms_by_type)::supports_form_type(formType)) {
         auto& list = this->forms_by_type[formType].forms;
         auto  it   = list.find(formID);
         if (it != list.end())
            return it->second;
      }
      return this->get_form(formID, ignore_none_stubs);
   }
   bool file_load_order::for_each_form_of_type(form_type formType, std::function<bool(form_stub*)> functor) {
      if (decltype(this->forms_by_type)::supports_form_type(formType)) {
         auto& list = this->forms_by_type[formType].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (!stub)
               continue;
            if (functor(stub))
               return true;
         }
      }
      return false;
   }

   bool file_load_order::active_file_has_name() const noexcept {
      if (!this->active_file)
         return false;
      if (this->active_file->get_filename().empty())
         return false;
      return true;
   }
   uint32_t file_load_order::active_file_form_count() const noexcept {
      uint32_t total = 0;
      for (auto& map : this->active_file_forms_by_type)
         total += map.forms.size();
      return total;
   }
   bool file_load_order::active_file_has_forms_of_type(enum form_type form_type) const noexcept {
      if (decltype(this->forms_by_type)::supports_form_type(form_type)) {
         auto& list = this->active_file_forms_by_type[form_type].forms;
         return !list.empty();
      }
      return false;
   }
   bare_form_id_t file_load_order::find_first_free_form_id_in_active_file(bare_form_id_t id) const noexcept {
      auto active_prefix = this->expected_active_file_prefix_post_save();
      if (active_prefix.is_undefined())
         return 0;
      auto min_id = active_prefix.min_form_id();
      auto max_id = active_prefix.max_form_id();
      id = id ? active_prefix.coerce_form_id(id) : min_id;
      //
      auto& map = this->active_file_forms.forms;
      if (map.size() >= (max_id - min_id)) // no form IDs available
         return 0;
      //
      auto& reservations = this->form_creation_request_info.reserved_formIDs;
      auto  guard        = std::lock_guard(this->form_creation_request_info.lock);
      //
      for (; id < max_id; ++id) {
         form_stub* stub = cobb::unordered_map_get_if_present(map, id);
         if (stub && !stub->is_none_stub())
            continue;
         auto it = std::find(reservations.begin(), reservations.end(), id);
         if (it != reservations.end())
            continue;
         if (stub && stub->is_none_stub()) {
            //
            // If we're capable of destroying this none-stub, then treat its form ID as available.
            // 
            // None-stubs can be destroyed if they come solely from the active file, and if all 
            // of the forms that reference them can be loaded.
            //
            bool available = false;
            for (auto& pair : stub->inbound) {
               auto& entry = pair.second;
               if (!entry.other)
                  continue;
               auto& other = *entry.other;
               if (!this->is_defined_in_active_file(other)) {
                  available = false;
                  break;
               }
               //
               // Test if we can load the form that references this none-stub. (TODO: These checks 
               // are duplicated from `form_stub::_load` and various functions it calls. We here skip 
               // some checks that really should've been assertions in the form-stub internals, e.g. 
               // checks that a record actually exists at the file offset that a non-hardcoded stub 
               // says it has. When we refactor form stubs during sustain, we really must replace 
               // some checks with assertions and, in general, make it easier to check whether a 
               // stub *can* be loaded without just going and loading it, as the function we usually 
               // use, `file_load_order::_can_destroy_none_stub`, does.)
               //
               if (!get_form_loader_function(other.form_type) || !can_construct_form_data(other.form_type)) {
                  available = false;
                  break;
               }
            }
            if (!available)
               continue;
         }
         return id;
      }
      return 0;
   }
   bool file_load_order::for_each_active_file_form(std::function<bool(form_stub*)> functor) {
      auto& list = this->active_file_forms.forms;
      for (auto& pair : list) {
         auto* stub = pair.second;
         if (!stub)
            continue;
         if (functor(stub))
            return true;
      }
      return false;
   }
   bool file_load_order::for_each_active_file_form_of_type(form_type form_type, std::function<bool(form_stub*)> functor) {
      if (decltype(this->forms_by_type)::supports_form_type(form_type)) {
         auto& list = this->active_file_forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (!stub)
               continue;
            if (functor(stub))
               return true;
         }
      }
      return false;
   }
   bool file_load_order::for_each_active_file_override_of_type(form_type form_type, std::function<bool(form_stub*)> functor) {
      if (!decltype(this->forms_by_type)::supports_form_type(form_type))
         return false;
      if (!this->active_file)
         return false;

      auto& list = this->active_file_forms_by_type[form_type].forms;
      for (auto& pair : list) {
         auto  id   = pair.first;
         auto* stub = pair.second;
         if (!stub)
            continue;
         if (stub->get_file_at_index(0) == this->active_file)
            continue;
         if (functor(stub))
            return true;
      }
      return false;
   }
   bool file_load_order::for_each_impossible_to_save_form(game g, std::function<bool(form_stub*)> functor) {
      if (g == game::skyrim_special)
         return false;
      for (auto& info : form_types) {
         if (!(info.flags & form_type_info::flag::is_skyrim_special))
            continue;
         if (this->for_each_active_file_form_of_type(info.form_type, functor))
            return true;
      }
      return false;
   }
   bool file_load_order::for_each_top_level_form_needing_save(enum form_type form_type, std::function<bool(form_stub*)> functor) {
      if (!decltype(this->forms_by_type)::supports_form_type(form_type))
         return false;
      //
      // A form needs to be saved if it has been edited during the current session, 
      // if it was defined in or overridden by the active file, or if any of these 
      // things are true for any of its child or descendant forms.
      //
      // In practice, this means that for form types that can have child forms, we 
      // need to iterate over all loaded forms to check for descendants that meet 
      // the criteria, whereas for form types that cannot have child forms, we only 
      // need to loop over active file forms.
      //
      if (form_type_info::lookup(form_type).flags & form_type_info::flag::can_have_children) {
         auto& list = this->forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (!stub)
               continue;
            if (stub->needs_save())
               if (functor(stub))
                  return true;
         }
      } else {
         //
         // - only loop over active file forms
         // - skip some of the checks in `form_stub::needs_save`
         //    - TODO: break that function apart into the general checks and parent checks; have the 
         //      latter call the former; choose which to invoke in this function
         //
         auto& list = this->active_file_forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (!stub)
               continue;
            if (stub->is_edited() || stub->file_list_includes(this->active_file))
               if (functor(stub))
                  return true;
         }
      }
      return false;
   }
   void file_load_order::get_active_file_name(std::filesystem::path& out) const noexcept {
      out.clear();
      if (this->active_file)
         out = this->active_file->get_filename();
   }
   bool file_load_order::has_active_file() const noexcept {
      return this->active_file != nullptr;
   }
   bool file_load_order::is_defined_in_active_file(const form_stub& stub) const noexcept {
      if (stub.source_file_count() > 1)
         return false; // the form is an override or is overridden
      return stub.file_list_includes(this->active_file);
   }
   bool file_load_order::is_defined_or_overridden_in_active_file(const form_stub& stub) const noexcept {
      return stub.file_list_includes(this->active_file);
   }

   bool file_load_order::for_each_active_file_game_setting(std::function<bool(const loaded_game_setting&)> functor) {
      std::lock_guard guard(this->game_settings.lock);
      auto& map = this->game_settings.by_name;
      for (auto& pair : map) {
         auto& list = pair.second;
         if (list.empty()) // shouldn't happen, but eh
            continue;
         auto& entry = list.back();
         if (entry.source_file != this->active_file)
            continue;
         if (functor(entry))
            return true;
      }
      return false;
   }
   bool file_load_order::for_each_loaded_game_setting(std::function<bool(const loaded_game_setting&)> functor) {
      std::lock_guard guard(this->game_settings.lock);
      auto& map = this->game_settings.by_name;
      for (auto& pair : map) {
         auto& list = pair.second;
         if (list.empty()) // shouldn't happen, but eh
            continue;
         if (functor(list.back()))
            return true;
      }
      return false;
   }
   bool file_load_order::get_loaded_setting_by_name(const std::string& name, loaded_game_setting& out) const noexcept {
      std::string lowercase;
      lowercase.reserve(name.size());
      for (auto c : name)
         lowercase += tolower(c);
      //
      std::lock_guard guard(this->game_settings.lock);
      auto& map  = this->game_settings.by_name;
      if (!cobb::unordered_map_contains(map, lowercase))
         return false;
      auto& list = map.at(lowercase); // can't use operator[] when the map is (accessed as) const
      out = list.back();
      return true;
   }
   bool file_load_order::get_loaded_setting_by_name(const game_setting_definition& definition, loaded_game_setting& out) const noexcept {
      return this->get_loaded_setting_by_name(definition.name, out);
   }

   #pragma region Load order code for various form modification requests
   form_stub* file_load_order::create_form_of_type(form_type ft) {
      auto request = this->request_form_creation(ft);
      return this->commit_form_creation_request(request);
   }
   form_creation_request file_load_order::request_form_creation(form_type ft) noexcept {
      using exception  = exceptions::form_creation_failed;
      using error_code = exception::error_code;

      form_creation_request result(*this);
      result.form_type = ft;
      //
      if (is_valid_form_type(ft) && this->active_file) {
         auto prefix = this->expected_active_file_prefix_post_save();
         auto formID = prefix.coerce_form_id(this->active_file->header.nextFormID);
         
         auto  guard = std::lock_guard(this->form_creation_request_info.lock);
         auto& list  = this->form_creation_request_info.reserved_formIDs;
         {
            auto guard = std::lock_guard(this->forms.lock);
            if (std::find(list.begin(), list.end(), formID) != list.end()) // if the form ID is reserved, then we need to find a new one
               formID = 0;
            if (!(formID & 0x00FFFFFF) || this->get_form(formID, true) != nullptr) {
               formID = this->find_first_free_form_id_in_active_file();
            }
            if (this->prefs.new_forms_avoid_extended_esl_form_id_range && formID) {
               auto local_id = formID & 0x00FFFFFF;
               if (local_id < 0x800) {
                  auto alternative_id = this->find_first_free_form_id_in_active_file(0x00000800);
                  if (alternative_id)
                     formID = alternative_id;
               }
            }
         }
         if (formID)
            list.push_back(formID);

         result.formID = formID;
      }
      return result;
   }
   std::optional<exceptions::form_creation_error_code> file_load_order::would_form_creation_request_fail(const form_creation_request& request) noexcept {
      using error_code = exceptions::form_creation_error_code;

      if (&request.owner != this)
         return error_code::wrong_load_order;
      if (!is_valid_form_type(request.form_type))
         return error_code::invalid_form_type;
      if (!this->active_file)
         return error_code::no_active_file;
      if (!request.formID)
         return error_code::no_form_id_available;
      if (!can_construct_form_data(request.form_type) || !can_load_form_data(request.form_type))
         return error_code::unimplemented_form_type;

      {
         const auto& info = form_type_info::lookup(request.form_type);
         if (info.flags & form_type_info::flag::is_skyrim_special) {
            if (this->current_game != game::skyrim_special)
               return error_code::form_type_unavailable_in_current_game;
         }
      }

      // Can we destroy any occupying none-stub?
      if (form_stub* occupier = this->get_form(request.formID, false)) {
         assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
         if (!this->_can_destroy_none_stub(*occupier))
            return error_code::cannot_sever_references_to_none_stub;
      }

      if (request.child_of) {
         auto parent_type = request.child_of->form_type;
         auto child_type  = request.form_type;

         bool error = false;
         if (form_type_is_cell_child(child_type)) { // validate parent/child relationships
            error = parent_type != form_type::cell;
         } else if (child_type == form_type::cell) {
            error = parent_type != form_type::worldspace;
         } else if (child_type == form_type::topic_info) {
            error = parent_type != form_type::topic;
         } else {
            error = true;
         }
         if (error)
            return error_code::invalid_parent_child_relationship;
         
         if (child_type == form_type::cell) { // validate worldspace grid coordinates
            auto& grid_opt = request.cell_grid_coordinates;
            if (!grid_opt.has_value())
               return error_code::exterior_cell_must_have_grid_coordinates;
            auto& grid_coords = grid_opt.value();

            auto* existing = form_stub_helpers::get_worldspace_cell_by_grid(*request.child_of, grid_coords.x, grid_coords.y);
            if (existing)
               return error_code::exterior_grid_coordinates_already_taken;
         }
      } else {
         if (form_type_is_cell_child(request.form_type))
            return error_code::cannot_create_reference_with_no_parent_cell;
      }
      if (request.clone_of && request.form_type == form_type::cell) {
         if (request.child_of) {
            if (!request.clone_of->is_exterior_cell())
               return error_code::interior_cell_clone_cannot_have_parent;
         } else {
            if (request.clone_of->is_exterior_cell())
               return error_code::exterior_cell_clone_must_have_parent;
         }
      }

      return {};
   }
   form_stub* file_load_order::commit_form_creation_request(form_creation_request& request) {
      using exception  = exceptions::form_creation_failed;
      using error_code = exception::error_code;

      bare_form_id_t formID = request.formID;

      auto error = this->would_form_creation_request_fail(request);
      if (error.has_value()) {
         throw exception(error.value(), request);
      }

      if (form_stub* occupier = this->get_form(request.formID, false)) {
         assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
         assert(this->_can_destroy_none_stub(*occupier));
         this->_destroy_none_stub(*occupier);
      }

      loaded_forms::Form* loaded = nullptr;
      uint32_t record_flags = 0;
      //
      auto* stub = new form_stub;
      stub->form_type = request.form_type;
      if (!request.clone_of) {
         loaded_forms::Form::constructor_params fcp;
         fcp.stub = stub;
         //
         loaded = create_blank_loaded_form_by_type(request.form_type, fcp);
         assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
      }
      stub->form     = loaded;
      stub->_add_file(*this->active_file, 0);
      stub->formID   = formID;
      stub->editorID = request.editorID;
      stub->set_edited(true);
      //
      if (request.cell_grid_coordinates.has_value()) {
         auto& src = request.cell_grid_coordinates.value();
         stub->get_or_create_addenda().grid_position = cell_grid_position{
            .x = src.x,
            .y = src.y,
         };
      }
      //
      {
         auto guard = std::lock_guard(this->forms.lock);
         this->forms.forms[formID] = stub;
         this->forms_by_type[stub->form_type].forms[formID] = stub;
         this->active_file_forms.forms[formID] = stub;
         this->active_file_forms_by_type[stub->form_type].forms[formID] = stub;
      }
      //
      if (request.child_of)
         stub->set_parent_form(request.child_of);
      //
      if (request.clone_of) {
         auto flags = request.clone_of->get_record_flags();
         flags &= ~tes_file_record_header::flag::deleted;
         flags &= ~tes_file_record_header::flag::partial;
         flags &= ~tes_file_record_header::flag::compressed;
         stub->edit_record_flags(flags, true); // clone record flags
         //
         auto original = request.clone_of->load();
         if (original) {
            loaded = original->clone(*stub);
            assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
         }
      } else {
         loaded->setup(*this);
      }
      //
      if (this->active_file) {
         this->active_file->header.nextFormID = this->find_first_free_form_id_in_active_file(request.formID + 1);
      }
      //
      assert(this->_abandon_form_id_reservation(formID) && "Wait, did we just create a form stub for a form ID that wasn't reserved? That shouldn't have happened!");
      request.formID = 0;
      if (this->on_form_create)
         (this->on_form_create)(stub);
      return stub;
   }
   form_duplication_request file_load_order::request_form_duplication() {
      return form_duplication_request(*this);
   }
   form_deletion_request file_load_order::request_form_deletion(form_stub& target) {
      return form_deletion_request(*this, target);
   }
   form_renumber_request file_load_order::request_form_renumber(form_stub& stub, bare_form_id_t desiredID) {
      using exception  = exceptions::form_renumber_failed;
      using error_code = exception::error_code;

      form_renumber_request result(*this, stub);
      result.desiredID = desiredID;
      //
      if (stub.is_hardcoded()) {
         throw exception(error_code::form_is_hardcoded, stub);
      }
      if (desiredID == 0) {
         throw exception(error_code::form_id_is_zero, stub);
      }
      if (!this->active_file) {
         throw exception(error_code::no_active_file, stub);
      }
      if (!this->is_defined_in_active_file(stub)) {
         throw exception(error_code::form_is_not_from_active_file, stub);
      }
      assert(!this->files.empty());
      {
         const auto prefix = this->expected_active_file_prefix_post_save();
         if (desiredID > prefix.max_form_id()) {
            throw exception(error_code::form_id_is_out_of_bounds, stub);
         }
         if (!prefix.can_co_opt_hardcoded_range() && (desiredID & 0x00FFFFFF) <= dovah::max_hardcoded_form_id) {
            throw exception(error_code::form_id_is_in_hardcoded_range, stub);
         }
      }
      {
         auto dst_prefix = file_prefix::from_form_id(desiredID, this->current_game == game::skyrim_classic);
         if (this->index_of_prefix(dst_prefix) < 0) {
            throw exception(error_code::form_id_is_out_of_bounds, stub);
         }
      }
      //
      auto guard1 = std::lock_guard(this->forms.lock);
      auto guard2 = std::lock_guard(this->form_creation_request_info.lock);
      if (this->has_form(desiredID)) {
         throw exception(error_code::form_id_is_occupied, stub);
      }
      auto& list = this->form_creation_request_info.reserved_formIDs;
      if (std::find(list.begin(), list.end(), desiredID) != list.end()) {
         throw exception(error_code::form_id_is_reserved, stub);
      }
      list.push_back(desiredID);
      return result;
   }
   void file_load_order::commit_form_renumber_request(form_renumber_request& request) {
      using exception  = exceptions::form_renumber_failed;
      using error_code = exception::error_code;

      if (&request.owner != this)
         throw exception(error_code::wrong_load_order, request.target);
      
      bare_form_id_t desiredID = request.desiredID;
      bare_form_id_t oldID     = request.target.formID;
      if (!desiredID)
         return;
      if (form_stub* occupier = this->get_form(desiredID, false)) {
         assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
         if (occupier->is_none_stub()) {
            //
            // If a none-stub is taking the desired form ID, then we need to destroy it. 
            // However, we first need to check whether we *can* destroy it: does it come 
            // solely from the active file?
            //
            for (auto& pair : occupier->inbound) {
               auto& entry = pair.second;
               if (!entry.other)
                  continue;
               if (!this->is_defined_in_active_file(*entry.other)) {
                  auto ex = exception(error_code::cannot_inject_form_overtop_none_stub, request.target);
                  ex.details.none_stub = occupier;
                  throw ex;
               }
            }
            //
            // Okay, let's attempt destruction.
            //
            if (!this->_can_destroy_none_stub(*occupier)) {
               auto ex = exception(error_code::cannot_sever_references_to_none_stub, request.target);
               ex.details.none_stub = occupier;
               throw ex;
            }
            this->_destroy_none_stub(*occupier);
         }
      }
      //
      auto& stub = request.target;
      this->_renumber_form(stub, desiredID, true);
      //
      // Un-reserve the form ID:
      //
      this->_abandon_form_id_reservation(desiredID);
      //
      request.desiredID = 0;
      if (this->on_form_renumber)
         (this->on_form_renumber)(stub, oldID, desiredID);
   }
   game_setting_edit_request file_load_order::request_game_setting_change(bool automatic_id) {
      game_setting_edit_request result(*this, automatic_id ? game_setting_edit_request::form_id_policy::find_valid_id : game_setting_edit_request::form_id_policy::use_chosen_id);
      return result;
   }
   void file_load_order::commit_game_setting_change_request(game_setting_edit_request& request) {
      using exception  = exceptions::game_setting_value_change_failed;
      using error_code = exception::error_code;

      if (&request.owner != this)
         throw exception(error_code::wrong_load_order, request.setting.name);

      if (!request.desiredID) {
         if (request.policy == game_setting_edit_request::form_id_policy::find_valid_id) {
            this->set_reserved_form_id_for(request);
         } else {
            throw exception(error_code::form_id_is_zero, request.setting.name);
         }
      }
      std::string lowercase;
      lowercase.reserve(request.setting.name.size());
      for (auto c : request.setting.name)
         lowercase += tolower(c);
      //
      auto* definition = game_setting_definition::lookup(request.setting.name.c_str());
      auto& map  = this->game_settings.by_name;
      auto& list = map[lowercase];
      //
      loaded_game_setting* entry = nullptr;
      if (!list.empty()) {
         auto& back = list.back();
         if (back.source_file == this->active_file)
            entry = &back;
      }
      if (!entry) {
         entry = &list.emplace_back();
         entry->source_file = this->active_file;
         if (!definition) {
            entry->name = request.setting.name;
         } else {
            entry->name       = definition->name;
            entry->definition = definition;
         }
      }
      entry->set_value(request.setting.value);
      this->_renumber_game_setting(*entry, request.desiredID);
      //
      // Okay. The form stub is now squared away. Now, we need to un-flag the form ID as reserved.
      //
      if (request.reservedID)
         this->_abandon_form_id_reservation(request.desiredID);
   }
   game_setting_renumber_request file_load_order::request_game_setting_renumber() {
      game_setting_renumber_request result(*this);
      return result;
   }
   void file_load_order::commit_game_setting_renumber_request(game_setting_renumber_request& request) {
      using exception  = exceptions::game_setting_renumber_failed;
      using error_code = exception::error_code;

      if (&request.owner != this)
         throw exception(error_code::wrong_load_order, request.setting);
      if (!request.desiredID) {
         throw exception(error_code::form_id_is_zero, request.setting);
      }
      std::string lowercase;
      lowercase.reserve(request.setting.size());
      for (auto c : request.setting)
         lowercase += tolower(c);
      //
      auto& map  = this->game_settings.by_name;
      auto& list = map[lowercase];
      //
      loaded_game_setting* entry = nullptr;
      if (!list.empty()) {
         auto& back = list.back();
         if (back.source_file == this->active_file)
            entry = &back;
      }
      if (!entry) {
         throw exception(error_code::setting_is_not_in_active_file, request.setting);
      }
      this->_renumber_game_setting(*entry, request.desiredID);
      //
      // Okay. The form stub is now squared away. Now, we need to un-flag the form ID as reserved.
      //
      if (request.reservedID)
         this->_abandon_form_id_reservation(request.desiredID);
      //
      request.done = true;
   }
   //
   void file_load_order::set_reserved_form_id_for(game_setting_edit_request& request, bare_form_id_t desired) {
      using exception  = exceptions::game_setting_value_change_failed;
      using error_code = exception::error_code;

      if (&request.owner != this)
         throw exception(error_code::wrong_load_order, request.setting.name);

      auto  guard = std::lock_guard(this->form_creation_request_info.lock);
      auto& list  = this->form_creation_request_info.reserved_formIDs;
      if (request.reservedID) {
         if (request.desiredID == desired)
            return;
         this->_abandon_form_id_reservation(request.desiredID);
         request.desiredID = 0;
      }
      //
      if (desired) {
         if (std::find(list.begin(), list.end(), desired) != list.end()) {
            throw exception(error_code::form_id_is_reserved, request.setting.name);
         }
         //
         bool reserve = true;
         if (cobb::unordered_map_contains(this->forms.forms, desired)) {
            auto* stub = this->forms.forms[desired];
            if (stub) {
               if (stub->form_type != form_type::setting) {
                  throw exception(error_code::form_id_is_occupied, request.setting.name);
               }
               reserve = false;
            }
         }
         request.desiredID = desired;
         if (reserve)
            list.push_back(desired);
         //
         return;
      }
      //
      // Okay, we want to auto-select a form ID. First, let's see if there's already a form ID for this 
      // setting.
      //
      loaded_game_setting loaded;
      if (this->get_loaded_setting_by_name(request.setting.name, loaded)) {
         if (loaded.formID) {
            request.desiredID  = loaded.formID;
            request.reservedID = false;
            return;
         }
      }
      //
      // If we made it here, then no, so let's find a form ID.
      //
      auto prefix = this->expected_active_file_prefix_post_save();
      if (prefix.is_undefined()) {
         throw exception(error_code::no_active_file, request.setting.name);
      }
      assert(this->active_file && "How were we able to get the prefix of the active file when the pointer has been lost?"); // in case any code changes in the future
      auto formID = prefix.coerce_form_id(this->active_file->header.nextFormID);
      {
         auto guard = std::lock_guard(this->forms.lock);
         if (std::find(list.begin(), list.end(), formID) != list.end()) // if the form ID is reserved, then we need to find a new one
            formID = 0;
         if (!(formID & 0x00FFFFFF) || cobb::unordered_map_contains(this->forms.forms, formID)) {
            formID = this->find_first_free_form_id_in_active_file();
            if (!formID) { // no form ID available
               throw exception(error_code::no_form_id_available, request.setting.name);
            }
         }
      }
      //
      list.push_back(formID);
      request.desiredID  = formID;
      request.reservedID = true;
   }
   void file_load_order::set_reserved_form_id_for(game_setting_renumber_request& request, bare_form_id_t desired) {
      using exception  = exceptions::game_setting_renumber_failed;
      using error_code = exception::error_code;

      if (&request.owner != this)
         throw exception(error_code::wrong_load_order, request.setting);

      auto  guard = std::lock_guard(this->form_creation_request_info.lock);
      auto& list  = this->form_creation_request_info.reserved_formIDs;
      if (request.reservedID) {
         if (request.desiredID == desired)
            return;
         this->_abandon_form_id_reservation(request.desiredID);
         request.desiredID = 0;
      }
      //
      if (!desired) {
         request.desiredID = desired;
         return;
      }
      if (std::find(list.begin(), list.end(), desired) != list.end()) {
         throw exception(error_code::form_id_is_reserved, request.setting);
      }
      //
      bool reserve = true;
      if (cobb::unordered_map_contains(this->forms.forms, desired)) {
         auto* stub = this->forms.forms[desired];
         if (stub) {
            if (stub->form_type != form_type::setting) {
               auto ex = exception(error_code::form_id_is_occupied, request.setting);
               ex.details.occupying_stub = stub;
               throw ex;
            }
            reserve = false;
         }
      }
      request.desiredID = desired;
      if (reserve)
         list.push_back(desired);
   }
   //
   void file_load_order::abandon_form_id_reservation(form_creation_request& request) {
      if (&request.owner != this)
         return;
      if (!request.formID)
         return;
      this->_abandon_form_id_reservation(request.formID);
      request.formID = 0;
   }
   void file_load_order::abandon_form_id_reservation(form_renumber_request& request) {
      if (&request.owner != this)
         return;
      if (!request.desiredID)
         return;
      this->_abandon_form_id_reservation(request.desiredID);
      request.desiredID = 0;
   }
   void file_load_order::abandon_form_id_reservation(game_setting_edit_request& request) {
      if (&request.owner != this)
         return;
      if (!request.reservedID)
         return;
      request.reservedID = false;
      this->_abandon_form_id_reservation(request.desiredID);
   }
   void file_load_order::abandon_form_id_reservation(game_setting_renumber_request& request) {
      if (&request.owner != this)
         return;
      if (!request.reservedID)
         return;
      request.reservedID = false;
      this->_abandon_form_id_reservation(request.desiredID);
   }
   #pragma endregion
   
   bool file_load_order::for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor) {
      std::filesystem::path filename;
      for (auto* file : this->files) {
         filename = file->get_filename();
         if (functor(filename, file == this->active_file))
            return true;
      }
      return false;
   }

   void file_load_order::stub_flagged_as_edited(form_stub* stub) noexcept {
      this->active_file_forms.forms[stub->formID] = stub;
      auto& at = this->active_file_forms_by_type[stub->form_type];
      at.forms[stub->formID] = stub;
      //
      // NOTE: Merely flagging a form stub as edited SHOULD NOT result in the active file being 
      // added to the stub's source file list (if not already present). Currently, our handling 
      // of singleton forms depends on no such addition being made (refer to documentation for 
      // more information).
      //
   }

   file_load_order::form_id_status file_load_order::local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const {
      if (id == 0) {
         return form_id_status::valid;
      }
      if (game_feature_support::hardcoded_form_ids_ignore_record_id_prefix(this->current_game, file->header.file_version)) {
         auto id_to_test = id & 0x00FFFFFF;
         if (id_to_test <= max_hardcoded_form_id) {
            id = id_to_test;
            return form_id_status::valid;
         }
      } else {
         if (id <= max_hardcoded_form_id) {
            return form_id_status::valid;
         }
      }
      if (!file) {
         id = 0;
         return form_id_status::missing_master;
      }
      uint8_t local  = file->header.masters.size();
      uint8_t prefix = id >> 0x18;
      if (prefix == local) {
         auto file_prefix = this->file_prefix_for(*file);
         id = file_prefix.coerce_form_id(id);
         return form_id_status::valid;
      }
      if (prefix > local) {
         id = 0;
         return form_id_status::out_of_bounds;
      }
      auto& name        = file->header.masters[prefix].master;
      auto  file_prefix = this->file_prefix_for(name);
      if (file_prefix.is_undefined()) {
         id = 0;
         return form_id_status::missing_master;
      }
      id = file_prefix.coerce_form_id(id);
      return form_id_status::valid;
   }
   file_load_order::form_id_status file_load_order::local_formID_to_global_formID(form_stub* stub, uint32_t& out) const {
      auto* file = stub->get_file_at_index(-1);
      assert(file != nullptr);
      out = stub->formID;
      return local_formID_to_global_formID(file, out);
   }
   bare_form_id_t file_load_order::remap_formID_for_save(bare_form_id_t id) const noexcept {
      auto prefix = file_prefix::from_form_id(id, !this->is_light_plugin_support_enabled()); // just need this to see whether to use a light or heavy form ID
      if (prefix.is_undefined())
         return 0;
      auto index = this->index_of_prefix(prefix);
      if (index < 0 || index >= 0xFF)
         return 0;
      bare_form_id_t result = 0;
      if (prefix.is_light()) {
         result = (id & 0x00000FFF) | (index << 0x18);
      } else {
         result = (id & 0x00FFFFFF) | (index << 0x18);
      }
      return result;
   }

   tes_file_header* file_load_order::get_active_file_header() const noexcept {
      if (!this->active_file)
         return nullptr;
      return &this->active_file->header;
   }

   void file_load_order::save_active_file(std::filesystem::path requested_filename, const dovah::tes_file_writing::write_config& cfg, dovah::tes_file_writing::write_results& results) {
      //
      // The process of saving an active file is somewhat complex, due to the need to support 
      // both Skyrim Classic  and Skyrim Special,  as well as the  need to support converting 
      // files from either game to the other.
      //
      // To start with, we need to weed out the obvious errors: there must be an active file; 
      // another save or load operation can't currently be in progress; if the active file is 
      // implicit, then it needs a filename; and the active file can't have enough masters to 
      // shove its forms into the 0xFF slot.
      //
      // After that, we need to run  a few additional checks  centered around ESLs and cross-
      // game conversion. ESL save operations need to fail if any form IDs are outside of the 
      // valid range for ESLs. Cross-game conversion  needs to fail if the current load order 
      // is impossible in the target game:  conversions from Classic to Special would fail if 
      // the number of  masters is high enough for  the full load  order to overflow into the 
      // 0xFE slot; the reverse conversion would fail if the active file has any ESL masters.
      //
      // Once we've decided that we can save the  file, we need to perform several tasks, but 
      // the order in which  we perform them is very  important.  The list, first without any 
      // consideration given to order:
      //
      //  - We need to actually write out data to a temporary *.TES file.
      //
      //  - We need to close the active file's mapped view, so we can gain write access.
      //
      //  - We need to rename the *.TES file to overwrite any *.ES* file that may be present.
      //
      //  - We need to toggle whether the load order  supports ESLs, based on what game we're 
      //    saving content for.
      //
      //  - If we're changing whether the active file is an ESL, then we need to renumber all 
      //    of its forms in-memory, moving them from or to the 0xFE slot.
      //
      //  - We need to update  the active file's  in-memory header data,  such as its list of 
      //    dependencies.
      //
      //  - We need to attempt to  reopen the active file's mapped  view, so that editing can 
      //    continue.
      //
      //  - We need to update the file  offsets of all form_stubs  for forms that are defined 
      //    or overridden in the active file.
      //
      // There are a lot  of moving parts that we  need to be mindful of.  For example, if we 
      // are converting an ESL file to Skyrim  Classic, we can't disable the load order's ESL 
      // support straightaway, as that would cause  form ID prefix checks to fail: the checks 
      // would stop accounting for ESLs and thus treat active file form IDs as if they belong 
      // to the 255th file in the load order, since they're in slot 0xFE. Similarly, toggling 
      // ESL support would prevent us from getting  the active file's load order prefix prior 
      // to the save operation,  which would interfere with mass-renumbering  forms in memory 
      // if that's necessary;  as such, we need to  make sure that we  grab the active file's 
      // load order  prefix (for use in  distinguishing the active  file's new forms from its 
      // overrides) beforehand.
      //
      // In practice, the full save process is as follows:
      //
      //  - Check for obvious errors and fail if needed.
      //
      //  - If we're  converting across games,  check whether  we can toggle  ESL support and 
      //    fail if we can't.
      //
      //  - Grab the active file's current (file_prefix).
      //
      //  - Write to a temporary file.
      //
      //  - Toggle ESL support.
      //
      //  - Close the active file's mapped view, so that the existing file can be overwritten 
      //    if need be (i.e. saving changes to a file for the same game),  and so that we can 
      //    open the view to  a different file if  need be (i.e.  converting across games, or 
      //    simply saving-as-new).
      //
      //  - Rename the temporary file, overwriting any existing file.
      //
      //  - If we changed whether the  active file was an ESL,  then mass renumber all of its 
      //    forms. Use the (file_prefix) that we grabbed earlier to tell overrides apart from 
      //    forms that are actually created in the active file.
      //
      //  - Update the active file's in-memory header data.
      //
      //  - Reopen the mapped view for the active file. If this fails, then tell the frontend 
      //    that the save operation succeeded, but that editing cannot continue.
      //
      //  - If we were able to open the mapped file view, then update the file offsets on all 
      //    form_stubs for  forms defined or overridden  in the active file.  Delete any form 
      //    stubs that were defined in the active  file but didn't save, as well as any none-
      //    stubs that were referred to only by active file forms.
      //

      auto process = load_order_processes::file_save(*this);
      process.desired_filename = requested_filename.string();
      process.write_config = cfg;
      process.execute();
   }
}