#include "Tree.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Tree::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case components::object_bounds::subrecord:
               this->bounds.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case components::harvestable::subrecord_signature_ingredient:
            case components::harvestable::subrecord_signature_sound:
            case components::harvestable::subrecord_signature_percentages:
               this->harvestable.load(record, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;

            case 'CNAM':
               if (subrecord.size() != 0x30) // Same logic used by the game/CK loaders.
                  break;
               subrecord.read(this->tree_data.trunk.flexibility);
               subrecord.read(this->tree_data.branch_flexibility);
               subrecord.read(this->tree_data.trunk.amplitude);
               subrecord.read(this->tree_data.front.amplitude);
               subrecord.read(this->tree_data.back.amplitude);
               subrecord.read(this->tree_data.side.amplitude);
               subrecord.read(this->tree_data.front.frequency);
               subrecord.read(this->tree_data.back.frequency);
               subrecord.read(this->tree_data.side.frequency);
               subrecord.read(this->tree_data.leaf.flexibility);
               subrecord.read(this->tree_data.leaf.amplitude);
               subrecord.read(this->tree_data.leaf.frequency);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Tree::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      components::harvestable::use_info_state harvestable_uis;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region ACTI subrecords
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case components::object_bounds::subrecord: // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case components::harvestable::subrecord_signature_ingredient:
            case components::harvestable::subrecord_signature_sound:
            case components::harvestable::subrecord_signature_percentages:
               harvestable_uis.read(record);
               break;
         }
      }
      harvestable_uis.commit(uib);
   }
   void Tree::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Tree*)out;

      copy->bounds = this->bounds;
      copy->harvestable.clone_from(this->harvestable, *copy);
      copy->model.clone_from(this->model, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;

      copy->tree_data = this->tree_data;
   }
   void Tree::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      this->harvestable.save(record, intfc);
      {
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->tree_data.trunk.flexibility);
         subrecord.write(this->tree_data.branch_flexibility);
         subrecord.write(this->tree_data.trunk.amplitude);
         subrecord.write(this->tree_data.front.amplitude);
         subrecord.write(this->tree_data.back.amplitude);
         subrecord.write(this->tree_data.side.amplitude);
         subrecord.write(this->tree_data.front.frequency);
         subrecord.write(this->tree_data.back.frequency);
         subrecord.write(this->tree_data.side.frequency);
         subrecord.write(this->tree_data.leaf.flexibility);
         subrecord.write(this->tree_data.leaf.amplitude);
         subrecord.write(this->tree_data.leaf.frequency);
         subrecord.close();
      }
   }
   void Tree::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->harvestable.sever_outbound_references_to(other, *this);
   }
   void Tree::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      this->harvestable.clear(*this);

      this->name.reset();

      this->tree_data = {};
   }
}