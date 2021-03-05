#include "ObjectReference.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"
#include "factories/hardcoded.h"
#include "components/extra_data/enable_state_parent.h"
#include "components/extra_data/_factory.h"

namespace dovah::loaded_forms {
   void ObjectReference::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->position.x);
               subrecord.read(this->position.y);
               subrecord.read(this->position.z);
               subrecord.read(this->rotation.x);
               subrecord.read(this->rotation.y);
               subrecord.read(this->rotation.z);
               break;
            case 'ONAM':
               this->is_open = true;
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               subrecord.read(this->base_form);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               if (this->extra_data.load(record, intfc) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
                  intfc.log_load_warning(
                     detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
                  );
               }
               break;
         }
      }
   }
   /*static*/ void ObjectReference::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      auto&     extra_uib = components::extra_data_use_info_builder::get(uib);
      form_id_t base_form;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               decltype(script_data)::generate_use_info(subrecord, uib);
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               subrecord.read(base_form);
               break;
            case 'EDID': // editor ID
            case 'ONAM':
            case 'DATA':
               break;
            default:
               if (extra_uib->generate_use_info(record) == components::extra_data_load_result::unrecognized) {
                  //
                  // If execution reaches this spot, then the subrecord is not extra-data.
                  //
               }
               break;
         }
      }
      uib.add_outbound_reference(base_form, use_info_entry::flag::object_reference);
   }
   bool ObjectReference::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (ObjectReference*)out;
      //
      copy->extra_data.clone_from(this->extra_data, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      copy->base_form.set(*copy, this->base_form);
      copy->is_open  = this->is_open;
      copy->position = this->position;
      copy->rotation = this->rotation;
      return true;
   }
   bool ObjectReference::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      //
      auto& NAME = record.open_next_subrecord('NAME');
      NAME.write(this->base_form);
      NAME.close();
      //
      this->extra_data.save(record, intfc);
      //
      if (this->is_open) {
         record.open_next_subrecord('ONAM').close();
      }
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->position.x);
      DATA.write(this->position.y);
      DATA.write(this->position.z);
      DATA.write(this->rotation.x);
      DATA.write(this->rotation.y);
      DATA.write(this->rotation.z);
      DATA.close();
      //
      return true;
   }
   void ObjectReference::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->extra_data.sever_outbound_references_to(other, *this);
      //
      this->base_form.clear_if(*this, other);
   }
   void ObjectReference::_clear_impl() noexcept {
      this->extra_data.clear(*this);
      this->script_data.clear(*this);
      this->base_form.set(*this, nullptr);
      this->is_open = false;
      //this->position = { 0, 0, 0 }; // don't reset this as that might change the parent cell
      this->rotation = { 0, 0, 0 };
   }
   bool ObjectReference::_friendly_delete_impl(const file_load_order& load_order) noexcept {
      if (!this->is_working_copy)
         this->stub.edit_record_flags(form_flag::disabled, true);
      //
      // Make the reference an opposite enable state child of the PlayerRef.
      //
      auto* player_ref = load_order.get_form(hardcoded_form_ids::PlayerRef);
      assert(player_ref && "ObjectReference::_friendly_delete_impl: Why is the PlayerRef form not reachable by ID?");
      auto* extra = this->extra_data.get_or_create<components::extra::enable_state_parent>(components::extra_data_type::enable_state_parent);
      extra->flags = components::extra::enable_state_parent::flag::opposite;
      extra->ref.set(*this, player_ref);

      //
      // TODO: xEdit uses -30000 as its preferred Z-coordinate, and it makes any actors that 
      // this procedure is applied to persistent. Why the latter?
      //

      //
      this->position.z = -9999.0F;
      //
      return true;
   }
}