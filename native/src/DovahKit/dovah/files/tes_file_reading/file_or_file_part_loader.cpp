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
   }
   file_or_file_part_loader::file_or_file_part_loader(file_loader& self, lo_interface_t& i) : load_interface(i) {
      this->loader = &self;
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
      
      constexpr const size_t end_of_parent_topic_child_list =
         (std::string::npos == 0xFFFFFFFF) ? // (uint32_t)-1 has special meaning in the file format
            std::string::npos - 1
         :
            std::string::npos
      ;

      #pragma region INFO pre-handling
      size_t     insert_info_at = end_of_parent_topic_child_list;
      form_stub* parent_topic   = nullptr;
      if (stub.form_type == form_type::topic_info) {
         parent_topic = stub.get_parent_form();
         if (parent_topic && parent_topic->form_type != form_type::topic)
            parent_topic = nullptr;
      }
      #pragma endregion
      
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               if (!(form_type_info::lookup(stub.form_type).flags & form_type_info::flag::no_editor_id)) {
                  subrecord.read(stub.editorID);
               }
               break;
            case 'PNAM':
               if (parent_topic) {
                  assert(stub.form_type == form_type::topic_info);
                  //
                  form_reference_t formID;
                  subrecord.read(formID);
                  //
                  if (auto* stub = formID.get_form_stub()) {
                     insert_info_at = parent_topic->index_of_child_info(*stub);
                     if (insert_info_at == std::string::npos) {
                        insert_info_at = 0;
                     } else {
                        ++insert_info_at;
                     }
                  } else {
                     insert_info_at = end_of_parent_topic_child_list;
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
         parent_topic->_insert_child_topic_info({}, stub, insert_info_at);
      #pragma endregion
   }

   file_load_order& file_or_file_part_loader::get_load_order() const noexcept {
      return this->load_interface.owner;
   }
}