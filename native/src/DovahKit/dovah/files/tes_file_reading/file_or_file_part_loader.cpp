#include "file_or_file_part_loader.h"
#include "../file_load_order.h"
#include "file_loader.h"
#include "../../form_stub_addenda.h"

#include "../../exceptions/file_load_failed.h"
#include "../../notices/file_load_errors/parent_form_is_missing.h"
#include "../../notices/base_file_load_warning.h"

namespace dovah::tes_file_reading {
   file_or_file_part_loader::file_or_file_part_loader(file_loader& f) : load_interface(f.get_load_interface(*this)) {
      this->loader = &f;
      this->options.current_game = this->load_interface.owner.get_current_game();
   }
   file_or_file_part_loader::file_or_file_part_loader(file_loader& self, lo_interface_t& i) : load_interface(i) {
      this->loader = &self;
      this->options.current_game = this->load_interface.owner.get_current_game();
   }
   
   void file_or_file_part_loader::log_load_warning(notices::base_file_load_warning& notice) {
      auto& file = this->get_file_loader();
      notice.source_file = file.get_filename();
      this->load_interface.log_warning(notice);
   }

   form_stub* file_or_file_part_loader::make_stub_for_record() {
      auto& record = this->get_current_record();
      auto  stub   = new form_stub();
      stub->_add_file(this->get_file_loader(), record.header_pos(), record.flags());
      stub->formID    = record.formID();
      stub->form_type = form_type_info::signature_to_form_type(record.signature());
      return stub;
   }
   void file_or_file_part_loader::set_stub_parent(form_stub* stub, bare_form_id_t parentID) {
      if (!parentID) {
         stub->_set_parent_form_one_way({}, nullptr);
         return;
      }
      auto& lo     = this->get_file_loader().get_load_interface(*this).owner;
      auto* parent = lo.get_form(parentID);
      if (!parent) {
         auto error = std::make_unique<dovah::notices::file_load_errors::parent_form_is_missing>();
         error->filename    = this->get_file_loader().get_filename();
         error->file_offset = this->get_position();
         error->form = {
            .local_id = stub->formID,
            .type     = stub->form_type,
         };

         auto ex = dovah::exceptions::file_load_failed();
         ex.details.file_load_error = std::move(error);
         throw ex;
      }
      stub->_set_parent_form_one_way({}, parent);
   }
   bool file_or_file_part_loader::commit_stub(form_stub*& stub) {
      auto& file = this->get_file_loader();
      auto result = file_load_order::form_id_status::valid;
      try {
         result = this->load_interface.owner.accept_form_stub(stub);
      } catch (const dovah::exceptions::file_load_failed& ex) {
         delete stub;
         stub = nullptr;

         throw; // re-throw without object slicing
      }
      if (result == file_load_order::form_id_status::injected_partial) {
         //
         // The file load order already logged this one on its own and did not 
         // accept the stub, but it only needs to be a warning, not an error.
         //
         delete stub;
         stub = nullptr;
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

      #pragma region INFO pre-handling
      form_stub* parent_topic   = nullptr;
      if (stub.form_type == form_type::topic_info) {
         parent_topic = stub.get_parent_form();
         if (parent_topic && parent_topic->form_type != form_type::topic)
            parent_topic = nullptr;
      }
      #pragma endregion
      
      bool editor_id_seen = false;

      form_stub* previous_sibling_info = nullptr;
      bool info_is_appended = true;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               editor_id_seen = true;
               if (!(form_type_info::lookup(stub.form_type).flags & form_type_info::flag::no_editor_id)) {
                  subrecord.read(stub.editorID);
               }
               break;
            case 'PNAM':
               if (parent_topic) {
                  assert(stub.form_type == form_type::topic_info);
                  //
                  auto     pos = subrecord.offset();
                  uint32_t v;
                  if (subrecord.read(v) && v != 0xFFFFFFFF) {
                     info_is_appended = false;
                     subrecord.seek(pos);

                     form_reference_t formID;
                     subrecord.read(formID);
                     previous_sibling_info = formID.get_form_stub();
                  }
               }
               break;
            case 'XCLC':
               if (is_ext) {
                  auto& pos = stub.get_or_create_addenda().grid_position.emplace();
                  subrecord.read(pos.x);
                  subrecord.read(pos.y);
               }
               break;
            default:
               continue;
         }
      }

      if (!editor_id_seen) {
         //
         // Edge-case: losing record supplies an editor ID; winning record clears it.
         //
         stub.editorID.clear();
      }
      
      #pragma region INFO post-handling
      if (parent_topic && !stub.test_record_flags(tes_file_record_header::flag::deleted)) {
         bool is_active_file = this->get_load_order().get_active_file() == this->loader;
         if (info_is_appended) {
            parent_topic->get_or_create_addenda().ordered_children._insert_child_on_load({}, is_active_file, stub);
         } else {
            parent_topic->get_or_create_addenda().ordered_children._insert_child_on_load({}, is_active_file, stub, previous_sibling_info);
         }
      }
      #pragma endregion
   }

   file_load_order& file_or_file_part_loader::get_load_order() const noexcept {
      return this->load_interface.owner;
   }
}