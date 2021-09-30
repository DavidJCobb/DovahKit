#include "file_or_file_part_loader.h"
#include "../../notice_code_list.h"
#include "../file_load_order.h"
#include "file_loader.h"
#include "../../form_stub_addenda.h"

namespace dovah::tes_file_reading {
   file_or_file_part_loader::file_or_file_part_loader(file_loader& f) : load_interface(f.get_load_interface(*this)) {
      this->loader = &f;
   }
   file_or_file_part_loader::file_or_file_part_loader(file_loader& self, lo_interface_t& i) : load_interface(i) {
      this->loader = &self;
   }
   
   basic_reader::object_type file_or_file_part_loader::next_record_or_group() {
      if (this->last_error.is_defined())
         return object_type::none;
      auto result = basic_reader::next_record_or_group(); // call super
      if (this->last_error.is_defined()) {
         this->log_load_error(this->last_error);
         this->_reset_last_error();
      }
      return result;
   }
   bool file_or_file_part_loader::next_subrecord() {
      if (this->last_error.is_defined())
         return false;
      auto result = basic_reader::next_subrecord(); // call super
      if (this->last_error.is_defined()) {
         this->log_load_error(this->last_error);
         this->_reset_last_error();
      }
      return result;
   }

   void file_or_file_part_loader::log_load_warning(detailed_notice& n) {
      auto& file = this->get_file_loader();
      n.set_cause_file(file.get_filename());
      this->load_interface.log_load_warning(n);
   }
   void file_or_file_part_loader::log_load_error(detailed_notice& n) {
      auto& file = this->get_file_loader();
      n.set_cause_file(file.get_filename());
      this->load_interface.log_load_error(n);
      file.abort();
   }

   form_stub* file_or_file_part_loader::make_stub_for_record() {
      auto& record = this->get_current_record();
      auto  stub   = new form_stub();
      stub->_add_file(this->get_file_loader(), record.header_pos(), record.flags());
      stub->formID   = record.formID();
      stub->formType = form_type_info::signature_to_form_type(record.signature());
      return stub;
   }
   bool file_or_file_part_loader::set_stub_parent(form_stub* stub, bare_form_id_t parentID) {
      if (!parentID) {
         stub->_set_parent_form_one_way(nullptr);
         return true;
      }
      auto& lo     = this->get_file_loader().get_load_interface(*this).owner;
      auto* parent = lo.get_form(parentID);
      if (!parent) {
         detailed_notice error;
         error.code = notice_code::parent_form_is_missing;
         error.set_file_offset(this->get_position());
         error.cause_form.fixedID = 0;
         error.cause_form.localID = stub->formID;
         error.cause_form.type    = stub->formType;
         error.set_flag(detailed_notice::flag::has_cause_form);
         this->log_load_error(error);
         //
         return false;
      }
      stub->_set_parent_form_one_way(parent);
      return true;
   }
   bool file_or_file_part_loader::commit_stub(form_stub*& stub) {
      auto& file = this->get_file_loader();
      if (file.is_aborted())
         return false;
      auto result = this->load_interface.owner.accept_form_stub(stub);
      switch (result) {
         case file_load_order::form_id_status::missing_master: // <-- this one in particular can only happen if we failed to load a master, which implies that a file was edited between us checking the header and us loading it
         case file_load_order::form_id_status::out_of_bounds:
            {
               detailed_notice error;
               error.code = notice_code::form_id_is_out_of_bounds;
               if (result == file_load_order::form_id_status::missing_master) {
                  error.code = notice_code::form_id_is_inside_of_a_missing_master;
               }
               error.set_cause_file(file.get_filename());
               error.set_file_offset(this->get_position());
               error.cause_form.fixedID = 0;
               error.cause_form.localID = stub->formID;
               error.cause_form.type    = stub->formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->log_load_error(error);
               //
               delete stub;
               stub = nullptr;
            }
            return false;
         case file_load_order::form_id_status::null_is_not_allowed:
            {
               detailed_notice error;
               error.code = notice_code::zero_is_not_an_allowed_form_id;
               error.set_cause_file(file.get_filename());
               error.set_file_offset(this->get_position());
               error.cause_form.fixedID = 0;
               error.cause_form.localID = stub->formID;
               error.cause_form.type    = stub->formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->log_load_error(error);
               //
               delete stub;
               stub = nullptr;
            }
            return false;
         case file_load_order::form_id_status::form_type_mismatch:
            //
            // The file load order already logged this one on its own, but it 
            // can't abort the load process, so we'll do that.
            //
            this->get_file_loader().abort();
            {
               delete stub;
               stub = nullptr;
            }
            return false;
         case file_load_order::form_id_status::injected_partial:
            //
            // The file load order already logged this one on its own and did not 
            // accept the stub, but it only needs to be a warning, not an error.
            //
            {
               delete stub;
               stub = nullptr;
            }
            return false;
      }
      return stub != nullptr;
   }
   void file_or_file_part_loader::extract_high_value_subrecords_for_stub(form_stub& stub) {
      //
      // This function locates "high-value"  subrecords in a record,  and uses them to acquire and 
      // store any form data that should be held on the form stub or its addenda struct. This data 
      // includes:
      //
      // 
      // ===== EDITOR ID =========================================================================
      //
      // A string (ideally unique) which identifies the form stub in development contexts, such as 
      // in the Creation Kit or when using certain console commands. We store this directly on the 
      // form stub.
      //
      //
      // ===== CELL GRID COORDINATES =============================================================
      //
      // Grid coordinates (not unit coordinates) which  indicate where in a worldspace an exterior 
      // cell should be placed. We store this in the form stub addenda.
      //
      //
      // ===== TOPIC INFO PLACEMENT ==============================================================
      //
      // Topic Infos should be placed in a special list within their parent Topics. This list must 
      // be built during load, because it is influenced by the order in which infos load. Refer to 
      // the internal  documentation <topic infos' placements in topics' info lists.txt>  for more 
      // information.
      //
      //
      auto& record = this->get_current_record();
      bool  is_ext = stub.is_exterior_cell();
      //
      #pragma region INFO pre-handling
      size_t     insert_info_at = 0;
      form_stub* parent_topic = nullptr;
      if (stub.formType == form_type::topic_info) {
         parent_topic = stub.get_parent_form();
         if (parent_topic && parent_topic->formType != form_type::topic)
            parent_topic = nullptr;
      }
      #pragma endregion
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               if (!(form_type_info::lookup(stub.formType).flags & form_type_info::flag::no_editor_id)) {
                  subrecord.read(stub.editorID);
               }
               break;
            case 'PNAM':
               if (parent_topic) {
                  assert(stub.formType == form_type::topic_info);
                  //
                  form_reference_t formID;
                  subrecord.read(formID);
                  //
                  if (auto* stub = formID.get_form_stub()) {
                     insert_info_at = parent_topic->index_of_child_info(*stub) + 1;
                  } else {
                     insert_info_at = std::string::npos;
                  }
               }
               break;
            case 'XCLC':
               if (is_ext) {
                  int32_t x;
                  int32_t y;
                  if (!stub.addenda)
                     stub.addenda = new form_stub_addenda;
                  auto& g = stub.addenda->grid_coords;
                  subrecord.read(x);
                  subrecord.read(y);
                  g.x = x;
                  g.y = y;
                  stub.addenda->flags |= form_stub_addenda::flag::has_grid_coordinates;
               }
               break;
            default:
               continue;
         }
      }
      //
      #pragma region INFO post-handling
      if (parent_topic && !stub.test_record_flags(tes_file_record_header::flag::deleted))
         parent_topic->_insert_child_topic_info(stub, insert_info_at);
      #pragma endregion
   }

   file_load_order& file_or_file_part_loader::get_load_order() const noexcept {
      return this->load_interface.owner;
   }
}