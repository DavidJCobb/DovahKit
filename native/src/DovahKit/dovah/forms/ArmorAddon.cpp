#include "./ArmorAddon.h"
#include "./_common_cpp.h"

namespace dovah::loaded_forms {
   void ArmorAddon::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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

            case 'RNAM':
               if (auto& form = this->races.primary; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::race, subrecord.signature());
               break;
            case 'DNAM':
               subrecord.read(this->graphics[sex::male].priority);
               subrecord.read(this->graphics[sex::female].priority);
               subrecord.read(this->graphics[sex::male].flags);
               subrecord.read(this->graphics[sex::female].flags);
               subrecord.skip_bytes(2);
               subrecord.read(this->loudness);
               subrecord.skip_bytes(1);
               subrecord.read(this->weapon_adjust);
               break;

            #pragma region Gendered subrecords
            case 'MOD2':
            case 'MO2T':
            case 'MO2S':
               this->graphics[sex::male].models.third_person.load(subrecord, intfc);
               break;
            case 'MOD3':
            case 'MO3T':
            case 'MO3S':
               this->graphics[sex::female].models.third_person.load(subrecord, intfc);
               break;
            case 'MOD4':
            case 'MO4T':
            case 'MO4S':
               this->graphics[sex::male].models.first_person.load(subrecord, intfc);
               break;
            case 'MOD5':
            case 'MO5T':
            case 'MO5S':
               this->graphics[sex::female].models.first_person.load(subrecord, intfc);
               break;

            case 'NAM0':
               if (auto& form = this->graphics[sex::male].skin_texture.base; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::texture_set, subrecord.signature());
               break;
            case 'NAM1':
               if (auto& form = this->graphics[sex::female].skin_texture.base; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::texture_set, subrecord.signature());
               break;
            case 'NAM2':
               if (auto& form = this->graphics[sex::male].skin_texture.swap_list; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::formlist, subrecord.signature());
               break;
            case 'NAM3':
               if (auto& form = this->graphics[sex::female].skin_texture.swap_list; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::formlist, subrecord.signature());
               break;
            #pragma endregion

            case 'MODL':
               if (auto& form = this->races.additional.emplace_back(); subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::race, subrecord.signature());
               break;
            case 'SNDD':
               if (auto& form = this->footstep_sound; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::footstep_set, subrecord.signature());
               break;
            case 'ONAM':
               if (auto& form = this->art_object; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::art_object, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ArmorAddon::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      struct graphics_use_info {
         struct {
            form_id_t base;
            form_id_t swap_list;
         } skin_texture;
      };

      form_id_t art_object;
      form_id_t footstep_set;
      data_by_sex<graphics_use_info> graphics;
      struct {
         form_id_t primary;
         std::vector<form_id_t> additional;
      } races;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case components::biped_object::subrecord_signature_deprecated:
            case components::biped_object::subrecord_signature_modern:
               components::biped_object::generate_use_info(subrecord, uib);
               break;
            case 'RNAM':
               subrecord.read(races.primary);
               break;

            case 'MOD2':
            case 'MO2T':
            case 'MO2S':
            case 'MOD3':
            case 'MO3T':
            case 'MO3S':
            case 'MOD4':
            case 'MO4T':
            case 'MO4S':
            case 'MOD5':
            case 'MO5T':
            case 'MO5S':
               components::model_ts::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'NAM0':
               subrecord.read(graphics[sex::male].skin_texture.base);
               break;
            case 'NAM1':
               subrecord.read(graphics[sex::female].skin_texture.base);
               break;
            case 'NAM2':
               subrecord.read(graphics[sex::male].skin_texture.swap_list);
               break;
            case 'NAM3':
               subrecord.read(graphics[sex::female].skin_texture.swap_list);
               break;

            case 'MODL':
               subrecord.read(races.additional.emplace_back());
               break;
            case 'SNDD':
               subrecord.read(footstep_set);
               break;
            case 'ONAM':
               subrecord.read(art_object);
               break;
         }
      }

      uib.add_outbound_reference(art_object);
      uib.add_outbound_reference(footstep_set);
      for (auto& item : graphics) {
         uib.add_outbound_reference(item.skin_texture.base);
         uib.add_outbound_reference(item.skin_texture.swap_list);
      }
      uib.add_outbound_reference(races.primary);
      for (auto id : races.additional)
         uib.add_outbound_reference(id);
   }
   void ArmorAddon::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ArmorAddon*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->biped_object.clone_from(this->biped_object, *copy);
      copy->bounds = this->bounds;

      for (size_t i = 0; i < sex_count; ++i) {
         auto& src = this->graphics[i];
         auto& dst = copy->graphics[i];
         dst.flags    = src.flags;
         dst.priority = src.priority;
         dst.models.first_person.clone_from(src.models.first_person, *copy);
         dst.models.third_person.clone_from(src.models.third_person, *copy);
         dst.skin_texture.base.set(*copy, src.skin_texture.base);
         dst.skin_texture.swap_list.set(*copy, src.skin_texture.swap_list);
      }

      copy->races.primary.set(*copy, this->races.primary);
      copy_form_reference_list(*copy, copy->races.additional, this->races.additional);

      copy->art_object.set(*copy, this->art_object);
      copy->footstep_sound.set(*copy, this->footstep_sound);
      copy->loudness = this->loudness;
      copy->weapon_adjust = this->weapon_adjust;
   }
   void ArmorAddon::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->biped_object.save(record, intfc);
      record.write_formID_subrecord('RNAM', this->races.primary);
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->graphics[sex::male].priority);
         subrecord.write(this->graphics[sex::female].priority);
         subrecord.write(this->graphics[sex::male].flags);
         subrecord.write(this->graphics[sex::female].flags);
         subrecord.skip_bytes(2);
         subrecord.write(this->loudness);
         subrecord.skip_bytes(1);
         subrecord.write(this->weapon_adjust);
         subrecord.close();
      }
      this->graphics[sex::male].models.third_person.save(record, intfc, 'MOD2', 'MO2T', 'MO2S');
      this->graphics[sex::female].models.third_person.save(record, intfc, 'MOD3', 'MO3T', 'MO3S');
      this->graphics[sex::male].models.first_person.save(record, intfc, 'MOD4', 'MO4T', 'MO4S');
      this->graphics[sex::female].models.first_person.save(record, intfc, 'MOD5', 'MO5T', 'MO5S');
      record.write_formID_subrecord('NAM0', this->graphics[sex::male].skin_texture.base, true);
      record.write_formID_subrecord('NAM1', this->graphics[sex::female].skin_texture.base, true);
      record.write_formID_subrecord('NAM2', this->graphics[sex::male].skin_texture.swap_list, true);
      record.write_formID_subrecord('NAM3', this->graphics[sex::female].skin_texture.swap_list, true);
      for (auto& item : this->races.additional)
         record.write_formID_subrecord('MODL', item, true);
      record.write_formID_subrecord('SNDD', this->footstep_sound, true);
      record.write_formID_subrecord('ONAM', this->art_object, true);
   }
   void ArmorAddon::_clear_impl() noexcept {
      this->biped_object.clear(*this);
      this->bounds.clear();
      this->script_data.clear(*this);
      //
      for (auto& item : this->graphics) {
         item.flags    = 0;
         item.priority = 0;
         item.models.first_person.clear(*this);
         item.models.third_person.clear(*this);
         item.skin_texture.base.set(*this, nullptr);
         item.skin_texture.swap_list.set(*this, nullptr);
      }
      clear_form_reference_list(this->races.additional, *this);
      this->art_object.set(*this, nullptr);
      this->footstep_sound.set(*this, nullptr);
      this->races.primary.set(*this, nullptr);
      this->loudness      = {};
      this->weapon_adjust = {};
   }
   void ArmorAddon::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->biped_object.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      
      this->races.primary.clear_if(*this, other);
      remove_form_from_reference_list(this->races.additional, other, *this);

      for (auto& item : this->graphics) {
         item.models.first_person.sever_outbound_references_to(other, *this);
         item.models.third_person.sever_outbound_references_to(other, *this);
         item.skin_texture.base.clear_if(*this, other);
         item.skin_texture.swap_list.clear_if(*this, other);
      }

      this->art_object.clear_if(*this, other);
      this->footstep_sound.clear_if(*this, other);
   }
}