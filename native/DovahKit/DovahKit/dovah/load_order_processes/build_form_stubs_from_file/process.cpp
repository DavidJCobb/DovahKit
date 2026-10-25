#include "./process.h"
#include <bit>
#include <tuple> // tuple_size_v
#include "lu/arrays/make_filled.h"
#include "dovah/_passkeys/build_use_info_during_load.h"
#include "dovah/active_load_order.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_addendum_types/passkeys/ordered_child_collection.h"
#include "dovah/form_stub_addendum_types/ordered_child_collection.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/form_stubs/is_interior_cell.h"
#include "./impl/fail_if_form_fatally_misplaced.h"
#include "./impl/file_offset_of_exception.h"
#include "./threads/forms_of_type.h"
#include "./make_stub_from_record.h" // when indexing worldspaces
#pragma region Committing stubs
   #include "./check_and_report_override_legality.h"
   #include "./set_file_list_for_newest_singleton_stub.h"
   #include "./update_stub_addenda_on_commit.h"

   #include "dovah/notices/file_parse_warnings/esl_defined_an_interior_cell.h"
   #include "dovah/notices/file_parse_warnings/form_initial_record_is_partial.h"
   #include "dovah/notices/file_parse_warnings/the_game_doesnt_load_new_actor_value_infos.h"
#pragma endregion
#pragma region After stub is committed
   #include "./extract_high_value_subrecords_for_stub.h"
#pragma endregion

#include "dovah/exceptions/file_parse_errors/child_form_group_follows_wrong_parent.h"
#include "dovah/exceptions/file_parse_errors/child_form_group_specifies_bad_parent_id.h"
#include "dovah/exceptions/file_parse_errors/interior_cell_block_group_badly_nested.h"
#include "dovah/exceptions/file_parse_errors/interior_cell_block_has_no_parent_group.h"
#include "dovah/exceptions/file_parse_errors/orphaned_group.h"
#include "dovah/exceptions/file_parse_errors/record_not_in_any_group.h"
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/apparent_persistent_cell_not_flagged_as_persistent.h"
#include "dovah/notices/file_parse_warnings/esl_defined_interior_cell_is_overridden.h"
#include "dovah/notices/file_parse_warnings/game_setting_has_multiple_records_in_a_file.h"
#include "dovah/notices/file_parse_warnings/game_setting_name_is_unrecognized.h"
#include "dovah/notices/file_parse_warnings/game_setting_record_has_bad_form_id.h"
#include "dovah/notices/file_parse_warnings/game_setting_record_has_no_name.h"
#include "dovah/notices/file_parse_warnings/game_setting_wants_to_override_a_real_form.h"
#include "dovah/notices/file_parse_warnings/partial_info_override_has_different_parent.h"
#include "dovah/notices/file_parse_warnings/record_found_in_wrong_group.h"

#define FOR_EACH_THREAD_TYPE(X) \
   X(exterior_cells_and_children) \
   X(forms_of_type) \
   X(game_settings) \
   X(interior_cells_and_children) \
   X(persistent_cell_children) \
   X(topics_and_infos)

#define FOR_EACH_THREAD_TYPE_WITH_DELIM(X, DELIM) \
   X(exterior_cells_and_children) DELIM \
   X(forms_of_type) DELIM \
   X(game_settings) DELIM \
   X(interior_cells_and_children) DELIM \
   X(persistent_cell_children) DELIM \
   X(topics_and_infos)

namespace dovah::load_order_processes::build_form_stubs_from_file {
   #pragma region thread_collection
      process::thread_collection::thread_collection(const process_params& params, process& owner) :
         #define XCOMMA ,
         #define X(_typename_, ...) \
            _typename_(lu::arrays::make_filled<threads::_typename_, std::tuple_size_v<decltype(_typename_)>>(threads::initialization_params{params, owner}))
         FOR_EACH_THREAD_TYPE_WITH_DELIM(X, XCOMMA)
         #undef X
         #undef XCOMMA
      {
      }
      void process::thread_collection::start() {
         #define X(_typename_, ...) \
            for (auto& worker : this->_typename_) worker.start();
         FOR_EACH_THREAD_TYPE(X)
         #undef X
      }
      void process::thread_collection::wait() {
         #define X(_typename_, ...) \
            for (auto& worker : this->_typename_) worker.wait_for();
         FOR_EACH_THREAD_TYPE(X)
         #undef X
      }
      void process::thread_collection::rethrow_earliest_caught_exception() {
         std::pair<std::exception_ptr, size_t> earliest;

         auto _per_list = [&earliest](auto& list) {
            for (base_worker_thread& worker : list) {
               if (!worker.exception)
                  continue;
               const auto offset = impl::file_offset_of_exception(worker.exception);
               if (!earliest.first || offset < earliest.second) {
                  earliest.first  = worker.exception;
                  earliest.second = offset;
               }
            }
         };
         #define X(_typename_, ...) _per_list(this->_typename_);
         FOR_EACH_THREAD_TYPE(X)
         #undef X
         
         if (earliest.first) {
            std::rethrow_exception(earliest.first);
         }
      }
   #pragma endregion

   process::process(const process_params& params) :
      _params(params),
      _threads(params, *this),
      _source_file_is_active(&params.source_file == params.active_load_order.get_active_file())
   {
   }

   void process::execute() {
      const auto& callbacks   = this->_params.callbacks;
      auto&       source_file = this->_params.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);
      {
         tes::file_view::chunk_type ot;

         struct {
            uint8_t exterior_cells_and_children = 0;
            uint8_t forms_of_type               = 0;
            uint8_t game_settings               = 0;
            uint8_t interior_cells_and_children = 0;
            uint8_t persistent_cell_children    = 0;
            uint8_t topics_and_infos            = 0;
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
                  // In order to skip the group's contents, call `view.skip_current_group()` and then `continue`. In order 
                  // to enter the group's contents and parse them, `continue` without skipping the group.
                  //
                  case tes::group_type::world_children:
                     //
                     // Parse direct children of the worldspace (i.e. the persistent cell).
                     //
                     if (!last_worldspace) [[unlikely]] {
                        throw exceptions::file_parse_errors::child_form_group_follows_wrong_parent(
                           source_file,
                           group.position,
                           group,
                           nullptr
                        );
                     } else {
                        auto result = utils::form_ids::record_id_to_form_id(source_file, group.label);
                        if (!result.has_value()) [[unlikely]] {
                           using exception_type = exceptions::file_parse_errors::child_form_group_specifies_bad_parent_id;
                           using exception_code = exception_type::problem_type;

                           exception_code code;
                           switch (result.error()) {
                              using enum utils::form_ids::record_id_to_form_id_error;
                              case out_of_bounds:
                                 code = exception_code::record_id_is_out_of_bounds;
                                 break;
                              case missing_master:
                                 throw exceptions::file_load_errors::missing_master_detected_at_record_load(
                                    source_file.path(),
                                    source_file.header().masters[group.label >> 0x18].master,
                                    group.label
                                 );
                           }
                           throw exception_type(
                              source_file,
                              group.position,
                              group,
                              code,
                              form_type::worldspace,
                              group.label,
                              last_worldspace
                           );
                        }
                        if (result.value() != last_worldspace->form_id) [[unlikely]] {
                           throw exceptions::file_parse_errors::child_form_group_follows_wrong_parent(
                              source_file,
                              group.position,
                              group,
                              last_worldspace
                           );
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
                     if (auto* parent_group = view.parent_of_group(group)) {
                        throw exceptions::file_parse_errors::unexpected_nested_group(
                           source_file,
                           (*parent_group).position,
                           (*parent_group),
                           group.position,
                           group
                        );
                     } else {
                        throw exceptions::file_parse_errors::orphaned_group(
                           source_file,
                           group.position,
                           group
                        );
                     }
                     continue;
                  case tes::group_type::exterior_cell_block:
                     last_ext_block_y = group.label & 0xFFFF;
                     last_ext_block_x = group.label >> 0x10;
                     continue;
                  case tes::group_type::exterior_cell_sub_block:
                     if (!last_worldspace) {
                        throw exceptions::file_parse_errors::child_form_group_follows_wrong_parent(
                           source_file,
                           group.position,
                           group,
                           nullptr
                        );
                     }
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
                              source_file,
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
                  throw exceptions::file_parse_errors::record_not_in_any_group(
                     source_file,
                     record.file_offset_of_header(),
                     record.header()
                  );
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
               auto* stub = make_stub_from_record(source_file, record.file_offset_of_header(), record.header());
               if (stub->form_type == form_type::worldspace) {
                  last_worldspace = stub;
               }

               switch (group.type) {
                  case tes::group_type::world_children:
                     if (stub->form_type == form_type::cell) {
                        //
                        // Loading (probable) persistent cell.
                        //
                        stub->_set_parent_form_one_way({}, last_worldspace);
                        last_persistent_cell = stub;

                        if (!stub->test_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent)) {
                           stub->_amend_source_file_flag({}, source_file, tes::record_flag::persistent, true); // auto-correct
                           if (callbacks.on_warning) {
                              (callbacks.on_warning)(
                                 notices::file_parse_warnings::apparent_persistent_cell_not_flagged_as_persistent(
                                    source_file.path().filename().string(),
                                    *stub
                                 )
                              );
                           }
                        }
                     } else {
                        if (callbacks.on_warning) {
                           (callbacks.on_warning)(
                              notices::file_parse_warnings::record_found_in_wrong_group(
                                 source_file.path().filename().string(),
                                 group.position,
                                 group,
                                 last_worldspace,
                                 record.file_offset_of_header(),
                                 record.header(),
                                 stub
                              )
                           );
                        }
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
                     impl::fail_if_form_fatally_misplaced<form_type::none>(source_file, record, *stub);
                     if (callbacks.on_warning) {
                        (callbacks.on_warning)(
                           notices::file_parse_warnings::record_found_in_wrong_group(
                              source_file.path().filename().string(),
                              group.position,
                              group,
                              nullptr,
                              record.file_offset_of_header(),
                              record.header(),
                              stub
                           )
                        );
                     }
                  }
               }
            }
         }
      }

      this->_threads.start();
      this->_threads.wait();
      this->_threads.rethrow_earliest_caught_exception();
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

   void process::_commit_game_setting(
      lu::passkey<threads::game_settings>,
      size_t        record_offset,
      const tes::record_header& record_header,
      std::string&& setting_name,
      types::game_settings::managed_value_variant&& setting_value
   ) {
      auto&       alo         = this->_params.active_load_order;
      const auto& callbacks   = this->_params.callbacks;
      auto&       source_file = this->_params.source_file;

      bare_form_id record_id = record_header.formID;
      bare_form_id form_id   = 0;
      {
         auto result = utils::form_ids::record_id_to_form_id(source_file, record_id);
         if (result.has_value()) {
            form_id = result.value();
         } else {
            if (callbacks.on_warning) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::game_setting_record_has_bad_form_id(
                     source_file.path().filename().string(),
                     static_cast<const std::string&>(setting_name), // casting from && to & is probably paranoid, but eh
                     record_id
                  )
               );
            }
         }
      }
      if (setting_name.empty()) {
         if (callbacks.on_warning) {
            (callbacks.on_warning)(
               notices::file_parse_warnings::game_setting_record_has_no_name(
                  source_file.path().filename().string(),
                  record_id,
                  form_id
               )
            );
         }
         return;
      }

      //
      // We want every GMST to have a form stub so that we can prevent form ID conflicts 
      // between GMSTs and forms. Technically, TESV.exe would allow a form to reuse the 
      // GMST's form ID if it loaded after the GMST, but given that we multi-thread our 
      // form loading, I don't want to have to try and account for that edge-case. Plus, 
      // other community-created tools can't be expected to cope with distinct entities 
      // having the same form ID.
      // 
      // We get the lock from out here because there are some correctness checks that we 
      // will need to do further down.
      //
      auto& fsc   = alo.get_form_stub_collection();
      auto  guard = std::lock_guard(fsc.forms_lock);
      if (form_id) {
         form_stub*& stub = fsc.forms[form_id];
         if (stub) {
            if (stub->form_type == form_type::setting) {
               stub->_add_source_file({}, const_cast<tes::file&>(source_file), (uint32_t)record_offset, (uint32_t)record_header.flags);
            } else {
               //
               // Don't allow GMSTs to override form IDs of other types; discard the GMST.
               //
               if (callbacks.on_warning) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::game_setting_wants_to_override_a_real_form(
                        source_file.path().filename().string(),
                        static_cast<const std::string&>(setting_name), // casting from && to & is probably paranoid, but eh
                        *stub
                     )
                  );
               }
               return;
            }
         } else {
            stub = new form_stub;
            stub->_add_source_file({}, const_cast<tes::file&>(source_file), (uint32_t)record_offset, (uint32_t)record_header.flags);
            stub->form_id   = form_id;
            stub->form_type = form_type::setting;
            fsc.forms_by_type[form_type::setting][form_id] = stub;
         }
         if (this->_source_file_is_active) {
            fsc.active_file_forms[form_id] = stub;
            fsc.active_file_forms_by_type[form_type::setting][form_id] = stub;
         }
      }
      
      //
      // Deal with the loaded game setting.
      //
      auto& gsc = alo.get_game_setting_collection();
      auto  insertion_result = gsc._store_game_setting(
         {}, // passkey
         source_file,
         setting_name,
         form_id,
         std::move(setting_value)
      );
      if (insertion_result.second) {
         if (callbacks.on_warning) {
            (callbacks.on_warning)(
               notices::file_parse_warnings::game_setting_has_multiple_records_in_a_file(
                  source_file.path().filename().string(),
                  insertion_result.first.name,
                  record_id,
                  form_id,
                  insertion_result.second
               )
            );
         }
         if (this->_source_file_is_active) {
            //
            // If the active file has redundant GMST records, then we should strip them out. We don't want to 
            // maintain multiple active-file form stubs for the same GMST, because that complicates the task 
            // of editing settings' values and the task of saving them into a file. Plus, this will give us 
            // consistent behavior with the Creation Kit: a game setting will have the last-seen form ID.
            // 
            // We don't want to strip redundant GMST records from dependencies, however, because we need to 
            // remember that all of the form IDs occupied by those records are, in fact, occupied; we need to 
            // know that we can't inject new active-file forms overtop those form IDs.
            // 
            // (This, incidentally, is why we hold the FSC lock for so much of this function.)
            //
            const bare_form_id old_form_id = insertion_result.second;
            //
            // Of course, none of this applies if the new redundant record didn't resolve to a valid form ID; 
            // in that case, there's no new stub to create, and we can stick with the old stub.
            //
            if (form_id) {
               auto* old_stub = fsc.forms[old_form_id];
               assert(!!old_stub);
               fsc.forms.erase(old_form_id);
               fsc.forms_by_type[form_type::setting].erase(old_form_id);
               fsc.active_file_forms.erase(old_form_id);
               fsc.active_file_forms_by_type[form_type::setting].erase(old_form_id);
               delete old_stub;
            }
         }
      }
      if (!insertion_result.first.definition) {
         if (callbacks.on_warning) {
            (callbacks.on_warning)(
               notices::file_parse_warnings::game_setting_name_is_unrecognized(
                  source_file.path().filename().string(),
                  insertion_result.first.name,
                  record_id,
                  form_id
               )
            );
         }
      }
   }

   void process::_commit_stub(
      form_stub*&               stub,
      size_t                    record_offset,
      const tes::record_header& record_header
   ) {
      auto& alo         = this->_params.active_load_order;
      auto& callbacks   = this->_params.callbacks;
      auto& source_file = this->_params.source_file;

      auto& fsc = alo.get_form_stub_collection();
      
      std::lock_guard<std::mutex> guard_for_all_forms(fsc.forms_lock);

      const bare_form_id record_id  = record_header.formID;
      const bare_form_id form_id    = stub->form_id;
      form_stub*         new_parent = utils::form_stubs::get_parent_form(*stub); // set by caller before calling us

      form_stub* existing_form = nullptr;
      {
         auto it = fsc.forms.find(form_id);
         if (it != fsc.forms.end())
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
         this->_fold_overriding_stub_into_overridden_stub(
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
               stub->_amend_source_file_flag({}, source_file, tes::record_flag::partial, false);
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
         fsc.forms[form_id] = stub;
         type[form_id] = stub;
      }
      //
      // We've either stored the stub as new, or treated it as an override (moving relevant data to the 
      // existing stub, deleting the new stub, and overwriting `stub` with a pointer to the existing stub). 
      //
      // Place the stub inside of the active file form maps, if appropriate.
      //
      if (this->_source_file_is_active) {
         fsc.active_file_forms[form_id] = stub;
         fsc.active_file_forms_by_type[stub->form_type][form_id] = stub;
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
   
   #pragma region Helpers for _commit_stub
      void process::_fold_overriding_stub_into_overridden_stub(
         form_stub&   overriding_stub,
         form_stub&   overridden_stub,
         size_t       override_record_offset,
         bare_form_id override_record_id
      ) {
         auto& callbacks       = this->_params.callbacks;
         auto& overriding_file = this->_params.source_file;

         form_stub* new_parent = utils::form_stubs::get_parent_form(overriding_stub); // set by caller before calling us

         if (overriding_stub.form_type == form_type::topic_info) {
            //
            // If the stub is a topic info and it's being re-parented by an override, then we need to 
            // update the form stub addenda for its old parent.
            //
            auto* old_parent = utils::form_stubs::get_parent_form(overridden_stub);
            if (old_parent && old_parent != new_parent) {
               if (auto* addenda = old_parent->addenda) {
                  addenda->ordered_children._remove_child_on_load({}, this->_source_file_is_active, overridden_stub);
               }
               if (callbacks.on_warning) {
                  if (overriding_stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::partial)) {
                     //
                     // A partial INFO override that re-parents the info? Oh, the game won't handle this 
                     // sanely at all. Refer to <topic infos' placement in topics' info lists.txt> in our 
                     // internal documentation for details on the chaos.
                     //
                     (callbacks.on_warning)(
                        notices::file_parse_warnings::partial_info_override_has_different_parent(
                           overriding_file.path().filename().string(),
                           override_record_id,
                           overridden_stub,
                           new_parent,
                           old_parent
                        )
                     );
                  }
               }
            }
         } else if (overriding_stub.form_type == form_type::cell) {
            if (overriding_stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent)) {
               auto* old_parent = utils::form_stubs::get_parent_form(overridden_stub);
               if (old_parent && old_parent != new_parent) { // Re-parenting cells isn't supported and isn't a sane operation, but no harm in covering it here
                  auto* addenda = old_parent->addenda;
                  if (addenda && addenda->persistent_cell == &overridden_stub)
                     addenda->persistent_cell = nullptr;
               }
            }
            if (callbacks.on_warning) {
               if (utils::form_stubs::is_interior_cell(overridden_stub)) {
                  auto* overridden_file_ptr = overridden_stub.get_file_at_index(0);
                  if (overridden_file_ptr && overriding_file.is_light()) {
                     (callbacks.on_warning)(
                        notices::file_parse_warnings::esl_defined_interior_cell_is_overridden(
                           overriding_file.path().filename().string(),
                           override_record_id,
                           overridden_stub
                        )
                     );
                  }
               }
            }
         }
         assert(!overriding_stub.has_multiple_source_files()); // the input stub should've been read by ONE file
         overridden_stub._add_source_file({}, const_cast<tes::file&>(overriding_file), (uint32_t)override_record_offset, overriding_stub.get_source_file_info(0)->record_flags);
         overridden_stub._set_parent_form_one_way({}, new_parent);
      }
   #pragma endregion
}