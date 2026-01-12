#include "./Armor.h"
#include "./_common_cpp.h"

namespace dovah::loaded_forms {
   void Armor::copy_data_from_template_armor(form_stub& source) {
      auto template_loaded = source.load().ptr_cast<Armor>();
      if (!template_loaded)
         return;

      auto& src_data = *template_loaded;


      this->race.set(*this, src_data.race); // TESRaceForm
      this->weight = src_data.weight; // TESWeightForm
      {  // BGSDestructibleObjectForm
         auto& src_opt = src_data.destruction_data;
         auto& dst_opt = this->destruction_data;
         if (src_opt.has_value()) {
            if (!dst_opt.has_value()) {
               dst_opt.emplace();
            }
            dst_opt.value().clone_from(src_opt.value(), *this);
         } else {
            if (dst_opt.has_value()) {
               dst_opt.value().clear(*this);
               dst_opt = {};
            }
         }
      }
      {  // BGSPickupPutdownSounds
         this->sounds.take.set(*this, src_data.sounds.take);
         this->sounds.drop.set(*this, src_data.sounds.drop);
      }
      {  // TESBipedModelForm
         for (size_t i = 0; i < this->world_models.size(); ++i) {
            auto& src_wm = src_data.world_models[i];
            auto& dst_wm = this->world_models[i];
            dst_wm.model.clone_from(src_wm.model, *this);
            dst_wm.icons = src_wm.icons;
         }
         this->ragdoll_constraint_template = src_data.ragdoll_constraint_template;
      }
      this->equip_type.set(*this, src_data.equip_type); // BGSEquipType
      this->biped_object.clone_from(src_data.biped_object, *this); // BGSBipedObjectForm
      {  // BGSBlockBashData
         this->block_bash.alternate_material.set(*this, src_data.block_bash.alternate_material);
         this->block_bash.impact_data_set.set(*this, src_data.block_bash.impact_data_set);
      }
      this->keywords.clone_from(src_data.keywords, *this); // BGSKeywordForm
      this->description = src_data.description; // TESDescription

      copy_form_reference_list(*this, this->armor_addons, src_data.armor_addons);
   }

   void Armor::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

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
            case components::biped_object::subrecord_signature_deprecated:
            case components::biped_object::subrecord_signature_modern:
               this->biped_object.load(subrecord, intfc);
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               this->enchantable.load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;

            case 'MOD2':
            case 'MO2T':
            case 'MO2S':
               this->world_models[sex::male].model.load(subrecord, intfc);
               break;
            case 'ICON':
               subrecord.read(this->world_models[sex::male].icons.inventory);
               break;
            case 'MICO':
               subrecord.read(this->world_models[sex::male].icons.message);
               break;

            case 'MOD4':
            case 'MO4T':
            case 'MO4S':
               this->world_models[sex::female].model.load(subrecord, intfc);
               break;
            case 'ICO2':
               subrecord.read(this->world_models[sex::female].icons.inventory);
               break;
            case 'MIC2':
               subrecord.read(this->world_models[sex::female].icons.message);
               break;

            case 'YNAM':
               if (auto& dst = this->sounds.take; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'ZNAM':
               if (auto& dst = this->sounds.drop; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;

            case 'BMCT':
               subrecord.read(this->ragdoll_constraint_template);
               break;
            case 'ETYP':
               if (auto& dst = this->equip_type; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::equip_slot, subrecord.signature());
               break;
            case 'BIDS':
               if (auto& dst = this->block_bash.impact_data_set; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               break;
            case 'BAMT':
               if (auto& dst = this->block_bash.alternate_material; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::material_type, subrecord.signature());
               break;
            case 'RNAM':
               if (auto& dst = this->race; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::race, subrecord.signature());
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'MODL':
               {
                  auto& form = this->armor_addons.emplace_back();
                  if (subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::armor_addon, subrecord.signature());
               }
               break;
            case 'DATA':
               subrecord.read(this->value);
               subrecord.read(this->weight);
               break;
            case 'DNAM':
               subrecord.read(this->rating);
               break;
            case 'TNAM':
               if (auto& dst = this->template_armor; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::armor, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Armor::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t equip_type;
      struct {
         form_id_t alternate_material;
         form_id_t impact_data_set;
      } block_bash;
      form_id_t race;
      form_id_t template_armor;
      form_id_t take_sound;
      form_id_t drop_sound;
      std::vector<form_id_t> armor_addons;
      components::enchantable::use_info_state enchantable;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case components::biped_object::subrecord_signature_deprecated:
            case components::biped_object::subrecord_signature_modern:
               components::biped_object::generate_use_info(subrecord, uib);
               break;
            case 'MOD2':
            case 'MO2T':
            case 'MO2S':
            case 'MOD4':
            case 'MO4T':
            case 'MO4S':
               components::model_ts::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               enchantable.read(subrecord);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'ETYP':
               subrecord.read(equip_type);
               break;
            case 'RNAM':
               subrecord.read(race);
               break;
            case 'BIDS':
               subrecord.read(block_bash.impact_data_set);
               break;
            case 'BAMT':
               subrecord.read(block_bash.alternate_material);
               break;
            case 'YNAM':
               subrecord.read(take_sound);
               break;
            case 'ZNAM':
               subrecord.read(drop_sound);
               break;
            case 'MODL':
               subrecord.read(armor_addons.emplace_back());
               break;
            case components::object_bounds::subrecord:
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
            case 'DNAM':
               break;
            case 'TNAM':
               subrecord.read(template_armor);
               break;
         }
      }
      uib.add_outbound_reference(block_bash.alternate_material);
      uib.add_outbound_reference(block_bash.impact_data_set);
      uib.add_outbound_reference(equip_type);
      uib.add_outbound_reference(race);
      uib.add_outbound_reference(template_armor, decltype(Armor::template_armor)::use_info_flag);
      uib.add_outbound_reference(take_sound);
      uib.add_outbound_reference(drop_sound);
      for (auto id : armor_addons)
         uib.add_outbound_reference(id);
      enchantable.commit(uib);
      destruction_uib.done();
   }
   void Armor::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Armor*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->biped_object.clone_from(this->biped_object, *copy);
      copy->bounds = this->bounds;
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->enchantable.clone_from(this->enchantable, *copy);
      copy->keywords.clone_from(this->keywords, *copy);

      copy->name = this->name;
      copy->description = this->description;

      copy_form_reference_list(*copy, copy->armor_addons, this->armor_addons);
      copy->block_bash.alternate_material.set(*copy, this->block_bash.alternate_material);
      copy->block_bash.impact_data_set.set(*copy, this->block_bash.impact_data_set);
      copy->equip_type.set(*copy, this->equip_type);
      copy->race.set(*copy, this->race);
      copy->ragdoll_constraint_template = this->ragdoll_constraint_template;
      copy->rating = this->rating;
      copy->sounds.drop.set(*copy, this->sounds.drop);
      copy->sounds.take.set(*copy, this->sounds.take);
      copy->template_armor.set(*copy, this->template_armor);
      copy->value  = this->value;
      copy->weight = this->weight;
      for (size_t i = 0; i < this->world_models.size(); ++i) {
         auto& src = this->world_models[i];
         auto& dst = copy->world_models[i];
         dst.model.clone_from(src.model, *copy);
         dst.icons = src.icons;
      }
   }
   void Armor::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      this->enchantable.save(record, intfc);
      {
         auto& item = this->world_models[sex::male];
         item.model.save(record, intfc, 'MOD2', 'MO2T', 'MO2S');
         if (auto& str = item.icons.inventory; !str.empty())
            record.write_string_subrecord('ICON', str);
         if (auto& str = item.icons.message; !str.empty())
            record.write_string_subrecord('MICO', str);
      }
      {
         auto& item = this->world_models[sex::female];
         item.model.save(record, intfc, 'MOD4', 'MO4T', 'MO4S');
         if (auto& str = item.icons.inventory; !str.empty())
            record.write_string_subrecord('ICO2', str);
         if (auto& str = item.icons.message; !str.empty())
            record.write_string_subrecord('MIC2', str);
      }
      this->biped_object.save(record, intfc);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('YNAM', this->sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->sounds.drop, true);
      if (auto& v = this->ragdoll_constraint_template; !v.empty()) {
         record.write_string_subrecord('BMCT', v);
      }
      record.write_formID_subrecord('ETYP', this->equip_type, true);
      record.write_formID_subrecord('BIDS', this->block_bash.impact_data_set, true);
      record.write_formID_subrecord('BAMT', this->block_bash.alternate_material, true);
      record.write_formID_subrecord('RNAM', this->race, true);
      this->keywords.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      for (auto& form : this->armor_addons)
         record.write_formID_subrecord('MODL', form, true);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->value);
         subrecord.write(this->weight);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->rating);
         subrecord.close();
      }
      record.write_formID_subrecord('TNAM', this->template_armor, true);
   }
   void Armor::_clear_impl() noexcept {
      this->biped_object.clear(*this);
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->enchantable.clear(*this);
      this->keywords.clear(*this);
      this->script_data.clear(*this);
      //
      this->name.reset();
      this->description.reset();
      //
      clear_form_reference_list(this->armor_addons, *this);
      this->block_bash.alternate_material.set(*this, nullptr);
      this->block_bash.impact_data_set.set(*this, nullptr);
      this->equip_type.set(*this, nullptr);
      this->race.set(*this, nullptr);
      this->ragdoll_constraint_template.clear();
      this->rating = 0;
      this->sounds.take.set(*this, nullptr);
      this->sounds.drop.set(*this, nullptr);
      this->template_armor.set(*this, nullptr);
      this->value = 0;
      this->weight = 0;
      for (auto& item : this->world_models) {
         item.model.clear(*this);
         item.icons = {};
      }
   }
   void Armor::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->biped_object.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->enchantable.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      //
      remove_form_from_reference_list(this->armor_addons, other, *this);
      this->block_bash.alternate_material.clear_if(*this, other);
      this->block_bash.impact_data_set.clear_if(*this, other);
      this->equip_type.clear_if(*this, other);
      this->race.clear_if(*this, other);
      this->sounds.take.clear_if(*this, other);
      this->sounds.drop.clear_if(*this, other);
      this->template_armor.clear_if(*this, other);
      for (auto& item : this->world_models) {
         item.model.sever_outbound_references_to(other, *this);
      }
   }
}