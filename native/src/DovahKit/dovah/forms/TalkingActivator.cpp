#include "TalkingActivator.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void TalkingActivator::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case components::object_bounds::subrecord:
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
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'PNAM':
               this->marker_color.load(subrecord);
               break;
            case 'SNAM':
               subrecord.read(this->looping_sound);
               intfc.warn_if_ref_is_wrong_type(this->looping_sound, form_type::sound_descriptor, subrecord.signature());
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
            #pragma region TACT subrecords
            case 'VNAM':
               if (auto& form = this->voicetype; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::voicetype, subrecord.signature());
               break;
            #pragma endregion
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void TalkingActivator::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t interact_keyword; // ACTI/KNAM
      form_id_t sound_loop;       // ACTI/SNAM
      form_id_t water_type;       // ACTI/WNAM
      form_id_t voicetype;        // TACT/VNAM
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region ACTI subrecords
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM': // activation sound
               subrecord.read(sound_loop);
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
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case components::object_bounds::subrecord: // bounds
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
            case 'VNAM':
               subrecord.read(voicetype);
               break;
            #pragma endregion
         }
      }
      uib.add_outbound_reference(interact_keyword); // ACTI/KNAM
      uib.add_outbound_reference(sound_loop);       // ACTI/SNAM
      uib.add_outbound_reference(water_type);       // ACTI/WNAM
      uib.add_outbound_reference(voicetype);        // TACT/VNAM
      destruction_uib.done();
   }
   void TalkingActivator::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (TalkingActivator*)out;
      
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
         copy->looping_sound.set(*copy, this->looping_sound);
         copy->water_type.set(*copy, this->water_type);
         copy->interact_keyword.set(*copy, this->interact_keyword);
         copy->activation_verb = this->activation_verb;
         copy->activator_flags = this->activator_flags;
      #pragma endregion
      #pragma region TACT fields
         copy->voicetype.set(*copy, this->voicetype);
      #pragma endregion
   }
   void TalkingActivator::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      {  // Fields inherited from ACTI
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
         this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
         if (this->destruction_data.has_value())
            this->destruction_data.value().save(record, intfc);
         this->keywords.save(record, intfc);
         auto& PNAM = record.open_next_subrecord('PNAM');
         this->marker_color.save(PNAM);
         PNAM.close();
         record.write_formID_subrecord('SNAM', this->looping_sound, true);
         //
         // ACTI/VNAM would serialize here, except it's impossible for ACTI/VNAM to ever load 
         // because it's shadowed by TACT/VNAM.
         //
         record.write_formID_subrecord('WNAM', this->water_type, true);
         if (!this->activation_verb.empty()) {
            auto& RNAM = record.open_next_subrecord('RNAM');
            RNAM.write(this->activation_verb);
            RNAM.close();
         }
         auto& FNAM = record.open_next_subrecord('FNAM');
         FNAM.write(this->activator_flags);
         FNAM.close();
         record.write_formID_subrecord('KNAM', this->interact_keyword, true);
      }
      record.write_formID_subrecord('VNAM', this->voicetype);
   }
   void TalkingActivator::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      //
      this->looping_sound.clear_if(*this, other);
      this->water_type.clear_if(*this, other);
      this->interact_keyword.clear_if(*this, other);

      this->voicetype.clear_if(*this, other);
   }
   void TalkingActivator::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->keywords.clear(*this);
      this->name.reset();
      this->looping_sound.set(*this, nullptr);
      this->water_type.set(*this, nullptr);
      this->interact_keyword.set(*this, nullptr);
      this->activation_verb.reset();
      this->activator_flags = 0;

      this->voicetype.set(*this, nullptr);
   }
}