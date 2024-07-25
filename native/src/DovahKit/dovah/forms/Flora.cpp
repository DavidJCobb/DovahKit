#include "Flora.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Flora::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            #pragma region ACTI subrecords
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'PNAM':
               this->marker_color.load(subrecord);
               break;
            case 'VNAM':
               subrecord.read(this->activation_sound);
               intfc.warn_if_ref_is_wrong_type(this->activation_sound, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'WNAM':
               subrecord.read(this->water_type);
               intfc.warn_if_ref_is_wrong_type(this->water_type, form_type::water_type, subrecord.signature());
               break;
            case 'RNAM':
               subrecord.read(this->activation_verb);
               break;
            case 'FNAM':
               subrecord.read(this->activator_flags);
               break;
            case 'KNAM':
               subrecord.read(this->interact_keyword);
               intfc.warn_if_ref_is_wrong_type(this->interact_keyword, form_type::keyword, subrecord.signature());
               break;
            #pragma endregion
            #pragma region FLOR subrecords
            case 'PFIG':
               subrecord.read(this->ingredient);
               intfc.warn_if_ref_is_wrong_type(this->ingredient, std::array{ form_type::ingredient, form_type::leveled_item }, subrecord.signature());
               break;
            case 'PFPC':
               subrecord.read(this->chance_by_season.spring);
               subrecord.read(this->chance_by_season.summer);
               subrecord.read(this->chance_by_season.autumn);
               subrecord.read(this->chance_by_season.winter);
               break;
            case 'SNAM':
               subrecord.read(this->harvest_sound);
               intfc.warn_if_ref_is_wrong_type(this->harvest_sound, form_type::sound_descriptor, subrecord.signature());
               break;
            #pragma endregion
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Flora::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t interact_keyword; // ACTI/KNAM
      form_id_t sound_activate;   // ACTI/VNAM
      form_id_t water_type;       // ACTI/WNAM
      form_id_t harvest_sound;    // FLOR/SNAM
      form_id_t ingredient;       // FLOR/PFIG
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region ACTI subrecords
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'VNAM': // activation sound
               subrecord.read(sound_activate);
               break;
            case 'WNAM': // water type, for water activators
               subrecord.read(water_type);
               break;
            case 'KNAM': // interaction keyword
               subrecord.read(interact_keyword);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'PNAM': // marker color
            case 'RNAM': // override activation prompt text
            case 'FNAM': // extra flags
               break;
            #pragma endregion
            #pragma region FLOR subrecords
            case 'PFIG':
               subrecord.read(ingredient);
               break;
            case 'SNAM':
               subrecord.read(harvest_sound);
               break;
            case 'PFPC':
               break;
            #pragma endregion
         }
      }
      uib.add_outbound_reference(interact_keyword); // ACTI/KNAM
      uib.add_outbound_reference(sound_activate);   // ACTI/VNAM
      uib.add_outbound_reference(water_type);       // ACTI/WNAM
      uib.add_outbound_reference(harvest_sound);    // FLOR/SNAM
      uib.add_outbound_reference(ingredient);       // FLOR/PFIG
      destruction_uib.done();
   }
   void Flora::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Flora*)out;
      
      #pragma region ACTI fields
         copy->script_data.clone_from(this->script_data, *copy);
         copy->bounds = this->bounds;
         copy->model.clone_from(this->model, *copy);
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
         copy->keywords.clone_from(this->keywords, *copy);
         copy->name = this->name;
         copy->marker_color = this->marker_color;
         copy->activation_sound.set(*copy, this->activation_sound);
         copy->water_type.set(*copy, this->water_type);
         copy->interact_keyword.set(*copy, this->interact_keyword);
         copy->activation_verb = this->activation_verb;
         copy->activator_flags = this->activator_flags;
      #pragma endregion
      #pragma region FLOR fields
         copy->harvest_sound.set(*copy, this->harvest_sound);
         copy->ingredient.set(*copy, this->ingredient);
         copy->chance_by_season = this->chance_by_season;
      #pragma endregion
   }
   void Flora::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      this->keywords.save(record, intfc);
      record.write_formID_subrecord('SNAM', this->harvest_sound, true);
      record.write_formID_subrecord('VNAM', this->activation_sound, true);
      record.write_formID_subrecord('WNAM', this->water_type, true);
      if (!this->activation_verb.empty()) {
         auto& RNAM = record.open_next_subrecord('RNAM');
         RNAM.write(this->activation_verb);
         RNAM.close();
      }
      record.write_formID_subrecord('KNAM', this->interact_keyword, true);
   }
   void Flora::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      //
      this->activation_sound.clear_if(*this, other);
      this->water_type.clear_if(*this, other);
      this->interact_keyword.clear_if(*this, other);
   }
   void Flora::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->keywords.clear(*this);
      this->name.reset();
      this->activation_sound.set(*this, nullptr);
      this->water_type.set(*this, nullptr);
      this->interact_keyword.set(*this, nullptr);
      this->activation_verb.reset();
      this->activator_flags = 0;
   }
}