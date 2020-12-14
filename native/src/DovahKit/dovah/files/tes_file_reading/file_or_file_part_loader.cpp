#include "file_or_file_part_loader.h"
#include "../../notice_code_list.h"
#include "../file_load_order.h"
#include "file_loader.h"

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
   bool file_or_file_part_loader::commit_stub(form_stub& stub) {
      auto& file = this->get_file_loader();
      if (file.is_aborted())
         return false;
      auto result = this->load_interface.owner.accept_form_stub(&stub);
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
               error.cause_form.localID = stub.formID;
               error.cause_form.type    = stub.formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->log_load_error(error);
            }
            return false;
         case file_load_order::form_id_status::null_is_not_allowed:
            {
               detailed_notice error;
               error.code = notice_code::zero_is_not_an_allowed_form_id;
               error.set_cause_file(file.get_filename());
               error.set_file_offset(this->get_position());
               error.cause_form.fixedID = 0;
               error.cause_form.localID = stub.formID;
               error.cause_form.type    = stub.formType;
               error.set_flag(detailed_notice::flag::has_cause_form);
               this->log_load_error(error);
            }
            return false;
      }
      return true;
   }
   void file_or_file_part_loader::extract_high_value_subrecords_for_stub(form_stub& stub) {
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
      auto& record = this->get_current_record();
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

   file_load_order& file_or_file_part_loader::get_load_order() const noexcept {
      return this->load_interface.owner;
   }
}