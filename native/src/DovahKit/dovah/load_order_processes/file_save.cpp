#include "./file_save.h"
#include "../files/file_load_order.h"

#include "../files/tes_file_reading/file_loader.h" // loaded files
#include "../forms/Form.h"

#include "../exceptions/file_load_failed.h" // for if reopening the file post-save fails
#include "../exceptions/file_save_failed.h"
#include "../exceptions/form_deletion_failed.h"
#include "../exceptions/game_change_failed.h"
#include "../load_order_requests/form_deletion_request.h"

#include "../data/game/hardcoded_form_ids_ignore_record_id_prefix.h"

namespace dovah::load_order_processes {
   void file_save::execute() {
      using exception  = exceptions::file_save_failed;
      using error_code = exception::error_code;

      if (!active_load_order.active_file)
         throw exception(error_code::no_active_file);

      std::filesystem::path desired_filename = active_load_order.active_file->get_filename();
      if (!this->desired_filename.empty())
         desired_filename = this->desired_filename;
      if (desired_filename.empty())
         throw exception(error_code::no_filename_specified);

      file_load_order::_save_load_lock_guard save_load_lock_guard(active_load_order, file_load_order::save_load_type::is_saving);
      if (!save_load_lock_guard)
         throw exception(error_code::save_or_load_already_in_progress);
      
      if (active_load_order.files.size() > 0xFE)
         throw exception(error_code::file_has_too_many_dependencies);
      
      desired_filename = std::filesystem::path(active_load_order.base_path) / desired_filename;
      std::filesystem::path temporary_filename = desired_filename;
      {  // opening the file for writing will clear its contents (which is bad for the user and will break our reading/writing), so we want to ALWAYS write to a temporary file first!
         auto ext = temporary_filename.extension().string();
         if (_stricmp(ext.data(), ".tes") == 0) {
            //
            // This normally should never happen. The editor should never allow you to open a *.TES 
            // file directly. You can end up working with one e.g. if a save is successful but we are 
            // unable to replace the file being saved over, but when that happens, we shouldn't be 
            // updating the file_reader's stored filename, so that should still point to the old name.
            //
         } else {
            temporary_filename.replace_extension(".tes");
         }
      }
      
      _old_active_file_prefix = active_load_order.file_prefix_for(*active_load_order.active_file);
      _was_originally_light   = active_load_order.active_file->is_light();
      _save_as_light_plugin   = (this->write_config.file_flags & tes_file_flag::light) || _stricmp(desired_filename.extension().string().data(), ".esl") == 0;
      if (this->write_config.output_game != game::skyrim_special)
         _save_as_light_plugin = false;

      //
      // Double-check for any illegal form IDs.
      //
      float desired_file_version = this->write_config.use_file_version.value_or(active_load_order.active_file->header.file_version);
      {
         bool verify_all_ids_in_light_range           = (_save_as_light_plugin && !_was_originally_light);
         bool verify_no_cannibalizing_hardcoded_range = false;
         if (active_load_order.current_game != this->write_config.output_game) {
            auto min_ver_opt = game_feature_support::hardcoded_form_ids_ignore_record_id_prefix_until_file_version(this->write_config.output_game);
            if (min_ver_opt.has_value()) {
               auto min_ver = min_ver_opt.value();
               if (desired_file_version < min_ver) {
                  if (this->write_config.use_file_version.has_value()) {
                     throw exception(error_code::desired_file_version_does_not_support_co_opting_the_hardcoded_form_id_range);
                  }
                  desired_file_version = min_ver;
               }
            } else {
               verify_no_cannibalizing_hardcoded_range = true;
            }
         }
         if (verify_all_ids_in_light_range || verify_no_cannibalizing_hardcoded_range) {
            for (auto& pair : active_load_order.active_file_forms.forms) {
               auto id = pair.second->formID;
               if (verify_all_ids_in_light_range) {
                  if (id & 0x00FFF000)
                     throw exception(error_code::forms_out_of_esl_form_id_range);
               }
               if (verify_no_cannibalizing_hardcoded_range) {
                  auto remapped = active_load_order.remap_formID_for_save(id);
                  if ((remapped & 0x00FFFFFF) < 0x800) {
                     throw exceptions::game_change_failed(dovah::game_change_failure_reason::active_file_co_opts_the_hardcoded_form_id_range, active_load_order.current_game, this->write_config.output_game);
                  }
               }
            }
         }
      }
      if (active_load_order.current_game != this->write_config.output_game) {
         auto code = active_load_order._can_change_current_game(this->write_config.output_game, _was_originally_light != _save_as_light_plugin);
         if (code.has_value())
            throw exceptions::game_change_failed(code.value(), active_load_order.current_game, this->write_config.output_game);
      }
      
      writer_type writer(active_load_order, *active_load_order.active_file, this->write_config);
      writer.config.use_file_version = desired_file_version;
      writer.path = temporary_filename;
      writer.open();
      try {
         writer.write();
      } catch (const dovah::exceptions::file_save_failed& ex) {
         throw; // re-throw for now
      }
      {
         //
         // Okay, so we have successfully written the updated form data to a temporary file. Now, we 
         // need to do a few things: we need to close the active file's mapped file view; we need to 
         // replace the old file with our temporary one; and we need to reopen the mapped file view 
         // on the updated file. (The file view needs to be closed because it's shared read access; 
         // it *should* prevent the file from being modified.)
         //
         writer.close(); // so we can move the new file
         active_load_order.active_file->close(); // so we can replace the old file
         //
         active_load_order._change_current_game(this->write_config.output_game, _was_originally_light != _save_as_light_plugin);
         //
         if (!writer.post_save_rename(desired_filename)) {
            this->results.saved_to_temporary_file = true;
            this->results.filename = temporary_filename.filename().string();
         } else {
            this->results.filename = desired_filename.filename().string();
         }
         try {
            active_load_order.active_file->reopen();
         } catch (const exceptions::file_load_failed& ex) {
            //
            // We were unable to reopen the mapped file view after fully updating the active file, 
            // so we can't load form content for active file form stubs anymore. In other words, the 
            // save completed, but further editing is not possible.
            //
            throw exception(error_code::save_complete_but_reopen_failed);
         }
         //
         // The file was reopened successfully, so let's update our in-memory state to match the data 
         // that was saved out.
         //
         this->_post_save_form_id_remap(writer);
         writer.update_source_file_header();
         //
         // The last two steps, before editing can resume, involve updating all form stubs in memory. 
         // Stubs that were saved to the new file need to have their file offsets updated. Active file 
         // stubs that were NOT saved (e.g. forms that were lost during a conversion between games) 
         // need to be discarded. (We need to discard those forms because the file they were originally 
         // loaded from may have been replaced, and if it wasn't, then it still isn't in use anymore; 
         // as such, if those forms have been unloaded, we can't load their data into memory anymore.)
         //
         this->_post_save_form_stub_file_info_update(writer);
         //
         // Okay, that's the file data updated for all saved stubs. Now, we need to discard the 
         // stubs that couldn't be saved, as well as any none-stubs that can safely be discarded. 
         // In order to avoid invalidating iterators during a loop and crashing, we'll gather up 
         // all the stubs-to-discard into a vector, and then chuck 'em all in another loop.
         //
         this->_post_save_unsaved_form_delete(writer);
      }
      //
      // Oh, and if we switched the active file's "light" flag, then the frontend may need a heads-up.
      //
      if (_was_originally_light != _save_as_light_plugin) {
         if (active_load_order.on_mass_renumber)
            (active_load_order.on_mass_renumber)();
      }
   }

   void file_save::_post_save_form_id_remap(writer_type& writer) {
      if (this->_was_originally_light != this->_save_as_light_plugin) {
         //
         // We have changed whether the active file is a light plug-in, so we need to change all 
         // of its form IDs in memory.
         //
         auto new_prefix = active_load_order.expected_active_file_prefix_post_save(this->_save_as_light_plugin);
         //
         std::vector<form_stub*> stubs;
         for (auto& pair : active_load_order.active_file_forms.forms) {
            auto* stub = pair.second;
            if (!stub)
               continue;
            auto  id   = stub->formID;
            if (this->_old_active_file_prefix.contains_form_id(id))
               stubs.push_back(stub);
         }
         //
         for (auto* stub : stubs) {
            if (stub->formID < 0x800) // don't renumber hardcoded form IDs
               continue;
            active_load_order._renumber_form(*stub, new_prefix.coerce_form_id(stub->formID), false);
         }
         //
         // And of course, we need to keep the loaded game settings consistent, too.
         //
         for (auto& pair : active_load_order.game_settings.by_name) {
            auto& list = pair.second;
            for (auto& entry : list) {
               if (entry.source_file != active_load_order.active_file)
                  continue;
               if (entry.formID < 0x800) // don't renumber hardcoded form IDs
                  continue;
               entry.formID = new_prefix.coerce_form_id(entry.formID);
            }
         }
         //
         // And the file-writer's state, since that gets used to deal with forms that 
         // weren't saved and thus need to be deleted.
         //
         {
            std::unordered_map<bare_form_id_t, writer_type::form_stub_write_info> remapped_write_info;
            for (auto& pair : writer.fixup_data.form_stubs) {
               auto* stub = pair.second.stub;
               if (stub) {
                  remapped_write_info[stub->formID] = std::move(pair.second);
               }
            }
            writer.fixup_data.form_stubs = std::move(remapped_write_info);
         }
      }
   }
   void file_save::_post_save_form_stub_file_info_update(writer_type& writer) {
      for (auto& pair : writer.fixup_data.form_stubs) {
         auto& info = pair.second;
         auto* stub = info.stub;
         stub->_set_source_file_offset(*active_load_order.active_file, info.offset);
         stub->_modify_source_file_record_flags(*active_load_order.active_file, tes_file_record_header::flag::partial, info.partial);
         if (!stub->is_edited()) {
            //
            // If the form stub was written to the file despite not having been flagged as edited, 
            // it would be because a child/descendant form or parent/ancestor form was edited. 
            // We need to add this form to the active file form list.
            //
            active_load_order.active_file_forms.forms[stub->formID] = stub;
            active_load_order.active_file_forms_by_type[stub->form_type].forms[stub->formID] = stub;
         }
         stub->set_edited(false);
         //
         if (!info.sever_references_to.empty()) {
            //
            // According to this form's use info, it refers to other forms that we were unable to 
            // write to the final file. Those references will have been serialized as zero, so now 
            // we need to sever them in-memory: if the form is still loaded, then we need to update 
            // the loaded data, and either way, we also need to update the use info.
            //
            // One example of where this could happen: imagine that we're converting the active 
            // file from Skyrim Special to Skyrim Classic, and it contains a form list that has 
            // an entry for a VOLI form -- a type that can only exist in Skyrim Special. When 
            // writing, we'll serialize form ID 0 instead of the VOLI form ID. If the VOLI form is 
            // part of the active file, then we'll also discard it below using a form deletion 
            // request, and that will sever the references to it. However, if the VOLI belongs to 
            // one of the active file's masters, then it won't be deleted, so we need to sever the 
            // references to it here.
            //
            auto loaded = stub->get_content_if_loaded(); // use this to ensure it doesn't unload out from under us
            for (auto id : info.sever_references_to) {
               auto* target = active_load_order.get_form(id, false);
               assert(target && "Error during post-save cleanup: How does one of the saved forms have a dangling form-to-form reference with no target none-stub?");
               if (loaded)
                  loaded->sever_outbound_references_to(*target);
               stub->revoke_all_outbound_references_to(target);
            }
         }
      }
   }
   void file_save::_post_save_unsaved_form_delete(writer_type& writer) {
      using exception  = exceptions::file_save_failed;
      using error_code = exception::error_code;

      auto stubs_to_remove = this->_find_stubs_to_discard_post_save(writer);

      bool failed_to_delete_unsaved_form = false;
      bool failed_to_delete_none_stub    = false;

      for (auto* stub : stubs_to_remove) {
         auto type = stub->form_type;
         if (type == form_type::setting) // GMSTs are a special case. their form-stubs are just placeholders and do not retain meaningful information, file offsets included
            continue;
         if (form_type_info::lookup(type).flags & form_type_info::flag::is_singleton) {
            //
            // Only strip these forms if they originated from the active file (i.e. the active file including 
            // redundant instances of a singleton form, and subsequently only saving one instance with merged 
            // data). If a stub isn't stripped, then remove its "edited" flag (that won't have been done above).
            //
            if (!stub->file_list_includes(active_load_order.active_file)) {
               stub->set_edited(false);
               continue;
            }
         }
         if (active_load_order.on_form_loss)
            (active_load_order.on_form_loss)(*stub); // ensure that the frontend can abandon any references it has to this stub and its loaded form data
         //
         bool is_none_stub = stub->is_none_stub();
         try {
            auto request = active_load_order.request_form_deletion(*stub); // this will also sever any uses of the form, which will prevent dangling stub pointers in any already-loaded "user" forms
            request.commit();
         } catch (const exceptions::form_deletion_failed& ex) {
            if (is_none_stub)
               failed_to_delete_none_stub = true;
            else
               failed_to_delete_unsaved_form = true;
         }
      }

      if (failed_to_delete_unsaved_form) {
         throw exception(error_code::unsaved_form_cleanup_failed);
      }
      if (failed_to_delete_none_stub) {
         throw exception(error_code::post_save_none_stub_cleanup_failed);
      }
   }

   std::vector<form_stub*> file_save::_find_stubs_to_discard_post_save(writer_type& writer) {
      std::vector<form_stub*> stubs_to_remove;
      for (auto& pair : active_load_order.active_file_forms.forms) {
         bare_form_id_t id = pair.first;
         if (!writer.fixup_data.form_stubs.contains(id))
            stubs_to_remove.push_back(pair.second); // removing can invalidate iterators, which would break this loop
      }
      size_t only_none_stubs_past_this_point = stubs_to_remove.size();
      for (auto& pair : active_load_order.forms_by_type[form_type::none].forms) {
         auto* stub = pair.second;
         if (!stub || !stub->is_none_stub())
            continue;
         //
         // There are two situations in which we can safely delete a none-stub post-save: 
         //
         //  - It was unreferenced at the start of the save process, and therefore still is 
         //    now.
         //
         //  - All references to it meet either of these two conditions:
         //
         //     - They are outbound from forms that were serialized to the file, and they 
         //       were therefore serialized as zero, tracked, and then severed as part of 
         //       the "fixup" loop above... which means that they *aren't* inbound 
         //       references anymore.
         //
         //     - They are outbound from forms that themselves need to be removed. (We can 
         //       use the (only_none_stubs_past_this_point) variable to test this faster, 
         //       because none-stubs should never be able to refer to each other.)
         //
         // Attempting to delete any other none-stub may result in non-active-file forms 
         // being wrongly flagged as "edited" as a result of the deletion. Those forms 
         // would then bake into the active file during subsequent saves.
         //
         bool can_delete = true;
         for (auto& pair : stub->inbound) {
            auto* user = pair.second.other;
            if (!user)
               continue;
            if (std::find(stubs_to_remove.begin(), stubs_to_remove.begin() + only_none_stubs_past_this_point, user) != stubs_to_remove.end()) // the referencing form is itself going to be discarded.
               continue;
            can_delete = false;
            break;
         }
         if (can_delete)
            stubs_to_remove.push_back(pair.second);
      }
      return stubs_to_remove;
   }
}