#include "MovementType.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void MovementType::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'SPED':
               subrecord.read(this->speeds.left.walk);
               subrecord.read(this->speeds.left.run);
               subrecord.read(this->speeds.right.walk);
               subrecord.read(this->speeds.right.run);
               subrecord.read(this->speeds.forward.walk);
               subrecord.read(this->speeds.forward.run);
               subrecord.read(this->speeds.back.walk);
               subrecord.read(this->speeds.back.run);
               subrecord.read(this->speeds.rotate_in_place.walk);
               subrecord.read(this->speeds.rotate_in_place.run);
               if (record.version() > 27) {
                  subrecord.read(this->speeds.rotate_while_moving);
               }
               break;
            case 'INAM':
               subrecord.read(this->anim_change_thresholds.directional);
               subrecord.read(this->anim_change_thresholds.movement_speed);
               subrecord.read(this->anim_change_thresholds.rotation_speed);
               break;
            case 'MNAM':
               subrecord.read(this->name);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MovementType::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void MovementType::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MovementType*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->speeds = this->speeds;
      copy->anim_change_thresholds = this->anim_change_thresholds;
      copy->name = this->name;
   }
   void MovementType::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);

      record.write_string_subrecord('MNAM', this->name);
      {
         auto& SPED = record.open_next_subrecord('SPED');
         SPED.write(this->speeds.left.walk);
         SPED.write(this->speeds.left.run);
         SPED.write(this->speeds.right.walk);
         SPED.write(this->speeds.right.run);
         SPED.write(this->speeds.forward.walk);
         SPED.write(this->speeds.forward.run);
         SPED.write(this->speeds.back.walk);
         SPED.write(this->speeds.back.run);
         SPED.write(this->speeds.rotate_in_place.walk);
         SPED.write(this->speeds.rotate_in_place.run);
         if (record.version() > 27) {
            SPED.write(this->speeds.rotate_while_moving);
         }
         SPED.close();
      }
      {
         auto& INAM = record.open_next_subrecord('INAM');
         INAM.write(this->anim_change_thresholds.directional);
         INAM.write(this->anim_change_thresholds.movement_speed);
         INAM.write(this->anim_change_thresholds.rotation_speed);
         INAM.close();
      }
   }
   void MovementType::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->name.clear();
      this->speeds = {};
      this->anim_change_thresholds = {};
   }
   void MovementType::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}