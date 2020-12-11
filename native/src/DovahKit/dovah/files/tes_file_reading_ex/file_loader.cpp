#include "file_loader.h"
#include "../../notice_code_list.h"
#include "file_threaded_part_loader_base.h"
#include "threads.h"

namespace dovah::tes_file_reading {
   file_loader::file_loader(interface_t& intfc) : load_interface(intfc) {
      for (size_t i = 0; i < threads::basic::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::basic(*this));
      }
      for (size_t i = 0; i < threads::dialogue::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::dialogue(*this));
      }
      for (size_t i = 0; i < threads::interior_cell::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::interior_cell(*this));
      }
      for (size_t i = 0; i < threads::worldspace_sub_block::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::worldspace_sub_block(*this));
      }
      for (size_t i = 0; i < threads::worldspace_persistent_cell_children::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::worldspace_persistent_cell_children(*this));
      }
      for (size_t i = 0; i < threads::game_setting::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::game_setting(*this));
      }
   }
   file_loader::~file_loader() {
      if (auto*& p = this->localization_data) {
         delete p;
         p = nullptr;
      }
      for (auto* thread : this->threads) {
         if (!thread)
            continue;
         #if _DEBUG
            if (thread->is_running())
               __debugbreak(); // why are you destroying a (file_loader) when one of its threaded readers is still running?
         #endif
         delete thread;
      }
      this->threads.clear();
   }

   #pragma region Threading
   float file_loader::assess_load_progress() const noexcept {
      size_t count = 0;
      float  total = 0.0F;
      for (auto* thread : this->threads) {
         if (!thread)
            return NAN;
         ++count;
         total += thread->assess_progress();
      }
      if (!count)
         return NAN;
      return total / count;
   }
   //
   bool file_loader::_start_threads() {
      for (auto* thread : this->threads)
         thread->start();
   }
   bool file_loader::_wait_for_threads() {
      for (auto* thread : this->threads)
         thread->wait_for();
   }
   //
   file_threaded_part_loader_base* file_loader::_get_nth_thread_of_type_impl(const std::type_info& ti, size_t n) {
      size_t i = 0;
      for (auto* thread : this->threads) {
         const auto& type = typeid(*thread);
         if (type == ti)
            if (n++ == i)
               return thread;
      }
      return nullptr;
   }
   #pragma endregion

   void file_loader::abort() noexcept {
      this->aborted = true;
   }
   bool file_loader::fetch_record_header(uint32_t pos, tes_file_record_header& out_header, uint32_t& record_decompressed_size) {
      this->reset_parse_state();
      this->set_position(pos);
      out_header = tes_file_record_header();
      if (this->next_record_or_group() == object_type::record) {
         out_header = this->_record.header;
         if (out_header.body_is_compressed()) {
            record_decompressed_size = this->_record.data.size();
         } else {
            record_decompressed_size = out_header.size;
         }
         return true;
      }
      return false;
   }
   std::string file_loader::get_filename() const noexcept {
      return this->path.filename().string();
   }
   file_load_order& file_loader::get_load_order() const noexcept {
      return this->load_interface.owner;
   }

   bool file_loader::load_record_at(uint32_t pos) {
      this->reset_parse_state();
      this->set_position(pos);
      return this->next_record_or_group() == object_type::record;
   }
   //
   file_loader::object_type file_loader::next_record_or_group() {
      this->_reset_last_error();
      auto result = basic_reader::next_record_or_group(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }
   bool file_loader::next_subrecord() {
      this->_reset_last_error();
      auto result = basic_reader::next_subrecord(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }

   #pragma region form_stub_build_interface
   form_stub* form_stub_build_interface::make_stub_for_record() {
      auto& record = this->reader.get_current_record();
      auto  stub   = new form_stub();
      stub->_add_file(this->owner, record.head_pos);
      stub->formID   = record.formID();
      stub->formType = form_type_info::signature_to_form_type(record.signature());
      if (record.flags() & tes_file_record_header::flag::deleted)
         stub->flags |= form_stub::flag::flagged_as_deleted;
      return stub;
   }
   bool form_stub_build_interface::commit_stub(form_stub& stub) {
      if (this->owner.aborted)
         return false;
      auto result = this->owner.get_load_order().accept_form_stub(&stub);
      switch (result) {
         case file_load_order::form_id_status::missing_master: // <-- this one in particular can only happen if we failed to load a master, which implies that a file was edited between us checking the header and us loading it
         case file_load_order::form_id_status::out_of_bounds:
            {
               detailed_notice error;
               error.code = notice_code::form_id_is_out_of_bounds;
               if (result == file_load_order::form_id_status::missing_master) {
                  error.code = notice_code::form_id_is_inside_of_a_missing_master;
               }
               error.set_cause_file(this->owner.get_filename());
               error.set_file_offset(this->owner.get_position());
               error.cause_form.fixedID = 0;
               error.cause_form.localID = stub.formID;
               error.cause_form.type    = stub.formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->owner.load_interface.log_load_error(error);
            }
            this->owner.abort();
            return false;
         case file_load_order::form_id_status::null_is_not_allowed:
            {
               detailed_notice error;
               error.code = notice_code::zero_is_not_an_allowed_form_id;
               error.set_cause_file(this->owner.get_filename());
               error.set_file_offset(this->owner.get_position());
               error.cause_form.fixedID = 0;
               error.cause_form.localID = stub.formID;
               error.cause_form.type    = stub.formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->owner.load_interface.log_load_error(error);
            }
            this->owner.abort();
            return false;
      }
      return true;
   }
   void form_stub_build_interface::extract_high_value_subrecords_for_stub(form_stub& stub) {
      if (form_type_info::lookup(stub.formType).flags & form_type_info::flag::no_editor_id)
         return;
      //
      struct _state {
         _state() = delete;
         enum {
            found_editor_id   = 0x01,
            found_cell_coords = 0x02,
         };
      };
      constexpr int found_all = _state::found_editor_id | _state::found_cell_coords;
      //
      int   state  = 0;
      auto& record = this->reader.get_current_record();
      bool  is_ext = stub.is_exterior_cell();
      if (!is_ext)
         state |= _state::found_cell_coords;
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               state |= _state::found_editor_id;
               subrecord.to_string(stub.editorID);
               break;
            case 'XCLC':
               state |= _state::found_cell_coords;
               subrecord.read(stub.groupInfo.gridX);
               subrecord.read(stub.groupInfo.gridY);
               break;
            default:
               continue;
         }
         if (state == found_all)
            return;
      }
      if (is_ext && !(state & _state::found_cell_coords)) {
         stub.flags |= form_stub::flag::missing_coordinates;
      }
   }
   #pragma endregion

   bool file_loader::load(const std::filesystem::path& new_path) {
      if (!new_path.empty())
         this->path = new_path;
      else if (this->path.empty()) {
         detailed_notice error;
         error.code = notice_code::no_filename_specified;
         this->load_interface.log_load_error(error);
         return false;
      }
      if (!this->_open_mapped_file()) // logs an error on its own
         return false;
      //
      if (!this->_load_header()) {
         detailed_notice error;
         error.code = notice_code::malformed_file;
         error.set_cause_file(this->get_filename());
         error.set_file_offset(this->get_position());
         this->load_interface.log_load_error(error);
         return false;
      }
      {
         object_type ot;
         struct {
            uint32_t basic    = 0;
            uint32_t dialogue = 0;
            uint32_t interior_cell   = 0;
            uint32_t world_sub_block = 0;
            uint32_t exterior_cell   = 0;
            uint32_t game_setting    = 0;
         } thread_indices;
         uint32_t last_worldspace_id = 0;
         uint32_t last_world_cell_id = 0;
         int16_t  last_ext_block_x = 0;
         int16_t  last_ext_block_y = 0;
         form_stub_build_interface fs_intfc = form_stub_build_interface(*this, *this);
         while (ot = this->next_record_or_group(), ot != object_type::none) {
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
            auto& group = this->get_current_group();
            if (ot == object_type::group) {
               switch (group.header.type) {
                  //
                  // In order to skip the group's contents, call (group.skip()) and then (continue). In order 
                  // to enter the group's contents and parse them, (continue) without skipping the group.
                  //
                  case group::type::world_children:
                     //
                     // Parse direct children of the worldspace (i.e. the persistent cell).
                     //
                     continue;
                  case group::type::cell_children:
                  case group::type::cell_persistent_children:
                  case group::type::cell_temporary_children:
                     assert(last_world_cell_id != 0 && "We should be ignoring these GRUPs when they don't appear after a worldspace's persistent cell!");
                     {
                        auto* loader = this->_get_nth_thread_of_type<threads::worldspace_persistent_cell_children>(thread_indices.exterior_cell);
                        assert(loader);
                        loader->add_group(last_world_cell_id, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::exterior_cell_block:
                     last_ext_block_y = group.header.label & 0xFFFF;
                     last_ext_block_x = group.header.label >> 0x10;
                     continue;
                  case group::type::exterior_cell_sub_block:
                     assert(last_worldspace_id && "Exterior Cell Block GRUP must follow a WRLD record.");
                     {
                        auto* loader = this->_get_nth_thread_of_type<threads::worldspace_sub_block>(thread_indices.world_sub_block);
                        assert(loader);
                        int16_t sub_x = group.header.label >> 0x10;
                        int16_t sub_y = group.header.label & 0xFFFF;
                        loader->add_group(last_worldspace_id, last_ext_block_x, last_ext_block_y, sub_x, sub_y, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::interior_cell_block:
                     {
                        {  // Error-checking.
                           auto parent = group.get_parent();
                           int  err = 0;
                           if (!parent)
                              err = 1;
                           else if (parent->header.type != group::type::forms_of_type)
                              err = 2;
                           else if (_byteswap_ulong(parent->header.label) != 'CELL')
                              err = 3;
                           if (err) {
                              detailed_notice error;
                              error.code = notice_code::interior_cell_block_group_badly_nested;
                              if (err == 1) {
                                 error.code = notice_code::interior_cell_block_has_no_parent_group;
                              }
                              error.set_cause_file(this->get_filename());
                              error.set_file_offset(this->get_position());
                              this->load_interface.log_load_error(error);
                              //
                              this->abort();
                              break;
                           }
                        }
                        auto* loader = this->_get_nth_thread_of_type<threads::interior_cell>(thread_indices.interior_cell);
                        assert(loader);
                        loader->add_group(group.header.label, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::forms_of_type:
                     last_world_cell_id = 0;
                     last_ext_block_x   = 0;
                     last_ext_block_y   = 0;
                     break;
                  default:
                     group.skip();
                     continue;
               }
               bool is_dialogue = false;
               bool is_gmst     = false;
               switch (_byteswap_ulong(group.header.label)) {
                  case 'CELL': // contents handled by the Interior Cell Block readers.
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
                  auto* loader = this->_get_nth_thread_of_type<threads::game_setting>(thread_indices.game_setting);
                  assert(loader);
                  loader->add_group(group.pos);
               } else if (is_dialogue) {
                  auto* loader = this->_get_nth_thread_of_type<threads::dialogue>(thread_indices.dialogue);
                  assert(loader);
                  loader->add_group(_byteswap_ulong(group.header.label), group.pos);
               } else {
                  auto* loader = this->_get_nth_thread_of_type<threads::basic>(thread_indices.basic);
                  assert(loader);
                  loader->add_group(_byteswap_ulong(group.header.label), group.pos);
               }
               //
               // Skip the group's actual content; the main thread only cares about locating the groups 
               // themselves and setting their contents up to be parsed on multiple threads.
               //
               group.skip();
               continue;
            }
            if (ot == object_type::record) {
               last_world_cell_id = 0;
               last_ext_block_x = 0;
               last_ext_block_y = 0;
               //
               // We should only hit records when we choose not to skip a group's contents. 
               // We use this to load worldspaces and their persistent/temporary cells in 
               // advance, so that we can multithread their contents a little more flexibly. 
               // We can assign a worldspace's descendant GRUPs to different workers without 
               // those workers having to care whether the worldspace is loaded; compare to 
               // topics and infos, which share a worker such that the worker has to load 
               // topics first (which is viable in that case because every topic has only 
               // one child GRUP, so multi-threading within a single topic isn't useful).
               //
               auto& record = this->get_current_record();
               form_type_t formType = form_type_info::signature_to_form_type(record.signature());
               auto* stub = fs_intfc.make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               switch (group.header.type) {
                  case group::type::world_children:
                     stub->groupInfo.parentFormID = last_worldspace_id;
                     last_world_cell_id = stub->formID;
                     break;
               }
               if (!fs_intfc.commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
                  continue;
               }
               fs_intfc.extract_high_value_subrecords_for_stub(*stub);
               if (record.signature() == 'WRLD')
                  last_worldspace_id = stub->formID;
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type_t group_type = form_type_info::signature_to_form_type(group_signature);
                     //
                     detailed_notice warning;
                     warning.code       = notice_code::record_found_in_wrong_top_level_group;
                     warning.cause_file = this->get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_file);
                     warning.set_cause_form(*stub);
                     warning.set_cause_signature(group_signature);
                     warning.set_cause_form_type(group_type);
                     //
                     this->load_interface.log_load_warning(warning);
                  }
               }
            }
         }
      }
      this->_start_threads();
      this->_wait_for_threads();
      //
      return !this->aborted;
   }
   void file_loader::close() {
      this->abort();
      this->_wait_for_threads();
      for (auto* thread : this->threads)
         if (thread)
            thread->_on_file_close();
      this->file = cobb::mapped_file();
   }

   bool file_loader::_open_mapped_file() {
      this->file = cobb::mapped_file();
      //
      /*//
      std::wstring foo;
      auto size = MultiByteToWideChar(CP_ACP, 0, this->path.data(), this->path.size(), foo.data(), 0);
      foo.resize(size);
      MultiByteToWideChar(CP_ACP, 0, this->path.data(), this->path.size(), foo.data(), size);
      //*/
      this->file.open(this->path.c_str());
      if (!this->file) {
         detailed_notice error;
         error.code    = notice_code::filesystem_error;
         error.type    = detailed_notice::notice_type::error;
         error.context = detailed_notice::notice_context::file_load;
         error.set_cause_file(this->get_filename());
         error.set_winapi_error_code(this->file.get_error());
         this->load_interface.log_load_error(error);
         //
         this->file = cobb::mapped_file();
         return false;
      }
      return true;
   }
   bool file_loader::_load_header() {
      if (this->next_record_or_group() != object_type::record) {
         detailed_notice error;
         error.code = notice_code::malformed_file; // Expected TES4 record; no record found.
         error.set_cause_file(this->get_filename());
         error.set_file_offset(this->get_position());
         this->load_interface.log_load_error(error);
         return false;
      }
      auto& r = this->get_current_record();
      if (r.signature() != 'TES4') {
         detailed_notice error;
         error.code = notice_code::malformed_file; // Expected TES4 record; got something else.
         error.set_cause_file(this->get_filename());
         error.set_file_offset(this->get_position());
         this->load_interface.log_load_error(error);
         return false;
      }
      {
         auto name = this->get_filename();
         this->header.flags = r.flags();
         if (name.size() > 4) {  // Force flags based on file extension.
            const char* extension = name.data() + name.size() - 4;
            if (_strnicmp(".esm", extension, 4) == 0) {
               this->header.flags |= flag::master;
            } else if (_strnicmp(".esl", extension, 4) == 0) {
               this->header.flags |= flag::master | flag::light;
            }
         }
      }
      this->header.record_version = r.version();
      //
      uint32_t last_subrecord = 0;
      while (auto& subrecord = r.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'HEDR': // required subrecord; TODO: fail if this isn't present
               if (!subrecord.is_in_bounds(12)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               subrecord.unchecked_read(this->header.file_version);
               subrecord.unchecked_read(this->header.record_and_group_count);
               subrecord.unchecked_read(this->header.nextFormID);
               break;
            case 'CNAM': // author/creator
               if (!subrecord.to_string(this->header.author)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               break;
            case 'SNAM': // description
               if (!subrecord.to_string(this->header.description)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               break;
            case 'MAST':
               if (last_subrecord == 'MAST') { // wrong; should be separated by 'DATA'
                  //dovah::logging::print_line("Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
               }
               {
                  this->header.masters.emplace_back();
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.to_string(last.master)) {
                     return false; // don't log an error here; caller should catch (return false) and log a catch-all error
                  }
                  if (this->header.masters.size() > 254) {
                     detailed_notice error;
                     error.code = notice_code::file_has_too_many_dependencies; // Expected TES4 record; no record found.
                     error.set_cause_file(this->get_filename());
                     error.set_file_offset(this->get_position());
                     this->load_interface.log_load_error(error);
                     return false;
                  }
                  //
                  // TODO: Have the load process fail if this function encounters any unexpected masters.
                  //
               }
               break;
            case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
               if (last_subrecord != 'MAST') {
                  detailed_notice error;
                  error.code = notice_code::malformed_file; // Expected TES4 record; no record found.
                  error.set_cause_file(this->get_filename());
                  error.set_file_offset(this->get_position());
                  this->load_interface.log_load_error(error);
                  return false;
               } else {
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.read(last.data)) {
                     //dovah::logging::print_line("Warning: failed to read the 'DATA' subrecord for master: %s", last.master.c_str());
                  }
               }
               break;
            case 'ONAM':
               //
               // TODO
               //
               this->header.details |= detail_flag::has_onam;
               break;
            case 'INTV':
               if (!subrecord.read(this->header.subINTV)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               this->header.details |= detail_flag::has_intv;
               break;
            case 'INCC':
               if (!subrecord.read(this->header.subINCC)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               this->header.details |= detail_flag::has_incc;
               break;
         }
         last_subrecord = subrecord.signature();
      }
      return true;
   }

   void file_loader::log_load_warning(const file_part_loader& from, detailed_notice& n) {
      this->load_interface.log_load_warning(n);
   }
   void file_loader::log_load_error(const file_part_loader& from, detailed_notice& n) {
      this->load_interface.log_load_error(n);
      this->abort();
   }
}