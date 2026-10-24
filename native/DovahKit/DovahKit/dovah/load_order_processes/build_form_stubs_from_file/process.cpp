#include "./process.h"
#include <bit>
#include "lu/arrays/make_filled.h"
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/form_stubs/is_interior_cell.h"
#include "./threads/forms_of_type.h"
#include "./threads/impl/fail_if_form_fatally_misplaced.h"
#include "./make_stub_from_record.h" // when indexing worldspaces
#pragma region Committing stubs
   #include "./check_and_report_override_legality.h"
   #include "./fold_overriding_stub_into_overridden_stub.h"
   #include "./set_file_list_for_newest_singleton_stub.h"
   #include "./update_stub_addenda_on_commit.h"

   #include "dovah/notices/file_parse_warnings/esl_defined_an_interior_cell.h"
   #include "dovah/notices/file_parse_warnings/form_initial_record_is_partial.h"
   #include "dovah/notices/file_parse_warnings/the_game_doesnt_load_new_actor_value_infos.h"
#pragma endregion
#pragma region After stub is committed
   #include "./extract_high_value_subrecords_for_stub.h"
#pragma endregion

#include "dovah/exceptions/file_parse_errors/interior_cell_block_group_badly_nested.h"
#include "dovah/exceptions/file_parse_errors/interior_cell_block_has_no_parent_group.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   process::process(const process_params& params) :
      _params(params),
      _threads({
         #define THREADS_OF_TYPE(_typename_) \
            ._typename_ = lu::arrays::make_filled<threads::_typename_, std::tuple_size_v<decltype(thread_collection::_typename_)>>(threads::initialization_params{params, *this})
         THREADS_OF_TYPE(exterior_cells_and_children),
         THREADS_OF_TYPE(forms_of_type),
         THREADS_OF_TYPE(game_settings),
         THREADS_OF_TYPE(interior_cells_and_children),
         THREADS_OF_TYPE(persistent_cell_children),
         THREADS_OF_TYPE(topics_and_infos),
         #undef THREADS_OF_TYPE
      })
   {
   }

   void process::execute() {
      tes::file_view view;
      view.set_target_file(&this->_params.source_file);
      {
         tes::file_view::chunk_type ot;

         struct {
            uint32_t exterior_cells_and_children = 0;
            uint32_t forms_of_type               = 0;
            uint32_t game_settings               = 0;
            uint32_t interior_cells_and_children = 0;
            uint32_t persistent_cell_children    = 0;
            uint32_t topics_and_infos            = 0;
         } thread_indices;
         #define ENQUEUE(_thread_typename_, ...) \
            {  \
               auto& workers = this->_threads._thread_typename_; \
               auto& worker  = workers[thread_indices._thread_typename_]; \
               worker.queue.push_back(__VA_ARGS__); \
               if (++thread_indices._thread_typename_ >= workers.size()) \
                  thread_indices._thread_typename_ = 0; \
            }

         form_stub*   last_worldspace      = nullptr;
         form_stub*   last_persistent_cell = nullptr;
         int16_t      last_ext_block_x     = 0;
         int16_t      last_ext_block_y     = 0;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            //
            // First, let's define some terms:
            //
            //  - Simple form type: A form type that cannot have child forms (i.e. no 
            //    nested GRUPs).
            //
            //  - Top-group: A top-level GRUP.
            //
            // Okay, so here's what we're doing:
            //
            //  - Top-groups of simple form types: Divide these across multiple threads. 
            //    Each thread will create FormStubs for each record in its assigned top-
            //    groups.
            //
            //  - Dialogue topics:
            //
            //     - These are handled by a single loader, which also handles their 
            //       child GRUPs and INFOs.
            //
            //  - Interior cells:
            //
            //     - Divide the interior cell block GRUPs across multiple threads. Each 
            //       thread will handle the CELL records themselves and any GRUPs nested 
            //       under the block GRUPs.
            //
            //  - Worldspace persistent cells:
            //
            //     - Load the cell here.
            //
            //     - Divide all of the cell's child GRUPs across multiple threads. Each 
            //       thread will handle the records nested under those GRUPs.
            //
            //  - Worldspaces:
            //
            //     - Load the worldspace here.
            //
            //     - Divide the worldspace's cell sub-block GRUPs across multiple threads. 
            //       Each thread will handle the CELL records themselves and any GRUPs 
            //       nested under the sub-block GRUPs.
            //
            if (ot == tes::file_view::chunk_type::group) {
               auto& group = *view.get_current_group();
               switch (group.type) {
                  //
                  // In order to skip the group's contents, call (view.skip_current_group()) and then (continue). In order 
                  // to enter the group's contents and parse them, (continue) without skipping the group.
                  //
                  case tes::group_type::world_children:
                     //
                     // Parse direct children of the worldspace (i.e. the persistent cell).
                     //
                     if (!last_worldspace) [[unlikely]] {
                        static_assert(false, "TODO: Error: world-children GRUP not immediately preceded by any WRLD record");
                     } else {
                        auto result = utils::form_ids::record_id_to_form_id(this->_params.source_file, group.label);
                        if (!result.has_value()) [[unlikely]] {
                           static_assert(false, "TODO: Error: world-children GRUP specifies invalid parent form ID");
                        }
                        if (result.value() != last_worldspace->form_id) [[unlikely]] {
                           static_assert(false, "TODO: Error: world-children GRUP not immediately preceded by the associated WRLD record");
                        }
                     }
                     continue;
                  case tes::group_type::cell_children:
                     assert(!!last_persistent_cell && "We should be ignoring these GRUPs when they don't appear directly after a worldspace's persistent cell!");
                     ENQUEUE(persistent_cell_children, { last_persistent_cell, (uint32_t)group.position });
                     view.skip_current_group();
                     continue;
                  case tes::group_type::cell_persistent_children:
                  case tes::group_type::cell_temporary_children:
                     static_assert(false, "TODO: Unexpected top-level or group");
                     continue;
                  case tes::group_type::exterior_cell_block:
                     last_ext_block_y = group.label & 0xFFFF;
                     last_ext_block_x = group.label >> 0x10;
                     continue;
                  case tes::group_type::exterior_cell_sub_block:
                     assert(last_worldspace && "Exterior Cell Block GRUP must follow a WRLD record.");
                     {
                        int16_t sub_x = group.label >> 0x10;
                        int16_t sub_y = group.label & 0xFFFF;
                        ENQUEUE(exterior_cells_and_children, { last_worldspace, { last_ext_block_x, last_ext_block_y }, { sub_x, sub_y }, (uint32_t)group.position });
                     }
                     view.skip_current_group();
                     continue;
                  case tes::group_type::interior_cell_block:
                     {  // Error-checking.
                        auto* parent = view.parent_of_group(group);
                        if (!parent) {
                           throw exceptions::file_parse_errors::interior_cell_block_has_no_parent_group(
                              this->_params.source_file,
                              group.position,
                              group
                           );
                        }
                        if (parent->type != tes::group_type::forms_of_type || std::byteswap(parent->label) != 'CELL') {
                           throw exceptions::file_parse_errors::interior_cell_block_group_badly_nested(
                              this->_params.source_file,
                              group.position,
                              group
                           );
                        }
                     }
                     ENQUEUE(interior_cells_and_children, { std::byteswap(group.label), (uint32_t)group.position });
                     view.skip_current_group();
                     continue;
                  case tes::group_type::forms_of_type:
                     last_worldspace      = nullptr;
                     last_persistent_cell = nullptr;
                     last_ext_block_x     = 0;
                     last_ext_block_y     = 0;
                     break;
                  default:
                     view.skip_current_group();
                     continue;
               }
               bool is_dialogue = false;
               bool is_gmst     = false;
               switch (std::byteswap(group.label)) {
                  case 'CELL': // interior cells are handled by the Interior Cell Block readers.
                  case 'WRLD': // forms handled here; children handled by the worldspace sub-block readers.
                     //
                     // Don't skip the group; we want to read at least some of the content inside of it. 
                     // However, don't assign the group to a simple-reader either.
                     //
                     continue;
                  case 'DIAL':
                     is_dialogue = true;
                     break;
                  case 'GMST':
                     is_gmst = true;
                     break;
               }
               if (is_gmst) {
                  ENQUEUE(game_settings, { (uint32_t)group.position });
               } else if (is_dialogue) {
                  ENQUEUE(topics_and_infos, { (uint32_t)group.position });
               } else {
                  ENQUEUE(forms_of_type, { std::byteswap(group.label), (uint32_t)group.position });
               }
               //
               // Skip the group's actual content; the main thread only cares about locating the groups 
               // themselves and setting their contents up to be parsed on multiple threads.
               //
               view.skip_current_group();
               continue;
            }
            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               auto* group_ptr = view.get_current_group();
               if (!group_ptr) {
                  if (record.signature() == 'TES4') [[likely]] {
                     continue;
                  }
                  static_assert(false, "TODO: error: (non-file-header) record not in any group");
               }
               auto& group = *group_ptr;

               last_persistent_cell = nullptr;
               last_ext_block_x     = 0;
               last_ext_block_y     = 0;
               //
               // We should only hit records when we choose not to skip a group's contents. 
               // We use this to load worldspaces and their persistent cells in advance, so 
               // that we can multithread their contents a little more flexibly. We can give 
               // a worldspace's descendant GRUPs to different workers without those workers 
               // having to care whether the worldspace is loaded; compare to topics and 
               // infos, which share a worker such that the worker has to load topics first 
               // (which is viable in that case because every topic has only one child GRUP, 
               // so multi-threading within a single topic isn't useful).
               //
               form_type formType = form_type_info::signature_to_form_type(record.signature());
               auto* stub = make_stub_from_record(
                  this->_params.source_file,
                  record.file_offset_of_header(),
                  record.header()
               );
               if (stub->form_type == form_type::worldspace) {
                  last_worldspace = stub;
               }

               switch (group.type) {
                  case tes::group_type::world_children:
                     if (stub->form_type == form_type::cell) {
                        //
                        // Loading (probable) persistent cell.
                        //
                        this->set_stub_parent(stub, last_worldspace);
                        last_persistent_cell = stub;

                        if (!stub->test_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent)) {
                           stub->edit_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent, true); // auto-correct
                           notices::file_load_warnings::apparent_persistent_cell_not_flagged_as_persistent notice(
                              stub->get_parent_form(),
                              *stub,
                              true
                           );
                           notice.source_file = this->get_file_loader().get_filename();
                           this->get_file_loader().get_load_interface(*this).log_warning(notice);
                        }
                     } else {
                        notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                        notice.top_level_group_label = std::byteswap(group.header.label);
                        notice.record = {
                           .local_id  = record.formID(),
                           .global_id = stub->form_id,
                           .signature = record.signature(),
                        };
                        notice.source_file = this->get_file_loader().get_filename();
                        this->get_file_loader().get_load_interface(*this).log_warning(notice);
                     }
                     break;
               }
               this->_commit_stub(stub, record.file_offset_of_header(), record.header());
               if (!stub) {
                  continue;
               }
               extract_high_value_subrecords_for_stub(this->_params.source_file, *stub, record);
               
               if (&group == &view.groups[0]) { // is this a top-level group?
                  lu::four_cc group_signature = std::byteswap(group.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     threads::impl::fail_if_form_fatally_misplaced(this->_params, record, *stub); // TODO: Move out of `threads::`
                     form_type group_type = form_type_info::signature_to_form_type(group_signature);
                     
                     notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                     notice.top_level_group_label = group_signature;
                     notice.record = {
                        .local_id  = record.formID(),
                        .global_id = stub->form_id,
                        .signature = record.signature(),
                     };
                     notice.source_file = this->get_file_loader().get_filename();
                     this->get_file_loader().get_load_interface(*this).log_warning(notice);
                  }
               }
            }
         }
      }
      this->_start_threads();
      this->_wait_for_threads();
      {
         const size_t no_offset = this->threads.size();

         size_t earliest_offset = no_offset;
         for (size_t i = 0; i < this->threads.size(); ++i) {
            auto* thread = this->threads[i];
            if (!thread->exception.captured)
               continue;

            if (earliest_offset == no_offset) {
               earliest_offset = i;
            } else {
               if (thread->exception.thrown_at_file_offset < this->threads[earliest_offset]->exception.thrown_at_file_offset)
                  earliest_offset = i;
            }
         }
         if (earliest_offset != no_offset) {
            std::rethrow_exception(this->threads[earliest_offset]->exception.captured);
         }
      }
      //
      return !this->aborted;
   }

   void process::_done_parsing_stub(
      passkeys::accept_stub,
      form_stub*& stub,
      tes::record_view& record
   ) {
      try {
         this->_commit_stub(
            stub,
            record.file_offset_of_header(),
            record.header()
         );
      } catch (...) {
         delete stub;
         throw;
      }
      if (stub)
         extract_high_value_subrecords_for_stub(this->_params.source_file, *stub, record);
   }

   void process::_commit_stub(
      form_stub*&               stub,
      size_t                    record_offset,
      const tes::record_header& record_header
   ) {
      auto& alo         = this->_params.active_load_order;
      auto& callbacks   = this->_params.callbacks;
      auto& source_file = this->_params.source_file;

      const bool source_file_is_active = &source_file == alo.get_active_file();

      auto& fsc = alo.get_form_stub_collection();
      
      std::lock_guard<std::mutex> guard_for_all_forms(fsc.forms.lock);

      const bare_form_id record_id  = record_header.formID;
      const bare_form_id form_id    = stub->form_id;
      form_stub*         new_parent = utils::form_stubs::get_parent_form(*stub); // set by caller before calling us

      form_stub* existing_form = nullptr;
      {
         auto it = fsc.forms.forms.find(form_id);
         if (it != fsc.forms.forms.end())
            existing_form = it->second;
      }

      if (existing_form) { // is this an override?
         load_order_processes::build_form_stubs_from_file::check_and_report_override_legality(
            callbacks,
            *stub,
            *existing_form,
            record_offset,
            record_id
         );
         load_order_processes::build_form_stubs_from_file::fold_overriding_stub_into_overridden_stub(
            callbacks,
            source_file,
            source_file_is_active,
            *stub,
            *existing_form,
            record_offset,
            record_id
         );
         //
         // Delete the new stub, and overwrite the pointer (in this function and in our caller(s)) with 
         // the existing stub.
         //
         delete stub;
         stub = existing_form;
      } else {
         //
         // This is not an override.
         //
         if (stub->form_type == form_type::actor_value_info) {
            if (callbacks.on_warning) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::the_game_doesnt_load_new_actor_value_infos(
                     source_file.path().filename().string(),
                     stub->form_id,
                     record_header,
                     record_offset
                  )
               );
            }
         }
         if (stub->source_file_count() == 1) {
            if (stub->test_record_flags((uint32_t)1 << (size_t)tes::record_flag::partial)) {
               bool injected = stub->is_injected();
               if (callbacks.on_warning) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::form_initial_record_is_partial(
                        source_file.path().filename().string(),
                        stub->form_id,
                        record_header,
                        record_offset,
                        injected
                     )
                  );
               }
               if (injected) {
                  //
                  // Skyrim will skip a partial record if it is injected and not an override. We'll 
                  // do the same. It's tempting to retain the form or to create a none-stub, but 
                  // either approach would incorrectly prevent subsequent files from injecting over 
                  // the same form ID.
                  //
                  delete stub;
                  stub = nullptr;
                  return;
               }
               //
               // Strip the flag for a non-injected non-override, as the game would.
               //
               stub->file.flags &= ~((uint32_t)1 << (size_t)tes::record_flag::partial);
            }
            if (callbacks.on_warning) {
               if (utils::form_stubs::is_interior_cell(*stub) && source_file.is_light()) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::esl_defined_an_interior_cell(
                        source_file.path().filename().string(),
                        stub->form_id,
                        record_id
                     )
                  );
               }
            }
         }
         //
         // Insert the stub into the form maps:
         //
         auto& type = fsc.forms_by_type[stub->form_type];
         fsc.forms.forms[form_id] = stub;
         {
            std::lock_guard<std::mutex> guard_for_form_type(type.lock);
            type.forms[form_id] = stub;
         }
      }
      //
      // We've either stored the stub as new, or treated it as an override (moving relevant data to the 
      // existing stub, deleting the new stub, and overwriting `stub` with a pointer to the existing stub). 
      //
      // Place the stub inside of the active file form maps, if appropriate.
      //
      if (source_file_is_active) {
         fsc.active_file_forms.forms[form_id] = stub;
         fsc.active_file_forms_by_type[stub->form_type].forms[form_id] = stub;
      }
      //
      // Finally, some finishing touches.
      //
      load_order_processes::build_form_stubs_from_file::update_stub_addenda_on_commit(
         *stub,
         new_parent
      );
      if (form_type_info::lookup(stub->form_type).flags & form_type_info::flag::is_singleton) {
         load_order_processes::build_form_stubs_from_file::set_file_list_for_newest_singleton_stub(
            fsc,
            callbacks,
            source_file,
            *stub,
            record_id
         );
      }
   }
}