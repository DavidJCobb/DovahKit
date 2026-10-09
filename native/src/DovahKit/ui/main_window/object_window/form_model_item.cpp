#include "./form_model_item.h"
#include "dovah/files/file_load_order.h"
#include "dovah/form_stub.h"
#include "editor/helpers/story_event_name.h"
#include "editor/subsystems/story_manager/core.h"

namespace ui::object_window {
   form_model_item::form_model_item(dovah::form_stub& stub) : stub(stub) {
      this->update();
   }

   void form_model_item::update() {
      this->editor_id = QString::fromUtf8(this->stub.get_editor_id());
      if (this->stub.form_type == dovah::form_type::story_event_node) {
         //
         // It's common for these forms to have no editor ID, and in fact the CK doesn't 
         // even let you give them one. Instead, the Object Window should identify them 
         // by their event typename.
         //
         auto& sm     = dovahkit::subsystems::story_manager::core::get_or_create();
         auto  et_opt = sm.event_type_for(this->stub);
         if (et_opt.has_value()) {
            auto et   = et_opt.value();
            auto name = editor_helpers::story_event_name(et);
            if (!name.isEmpty()) {
               this->editor_id = name;
            }
         }
      }
      this->form_id    = this->stub.formID;
      this->user_count = this->stub.inbound.size();
   
      this->is_active = false;
      if (!this->stub.test_record_flags(dovah::tes_file_record_header::flag::partial)) {
         if (this->stub.is_edited())
            this->is_active = true;
         else {
            this->is_active = this->stub.get_owning_load_order().is_defined_or_overridden_in_active_file(this->stub);
         }
      }

      this->is_injected = this->stub.is_injected();
      this->is_none     = this->stub.is_none_stub();
   }
   bool form_model_item::update_user_count() {
      auto updated = this->stub.inbound.size();
      if (this->user_count == updated)
         return false;
      this->user_count = updated;
      return true;
   }
}