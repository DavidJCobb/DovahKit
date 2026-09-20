#include "Furniture.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   Furniture::marker* Furniture::get_marker(uint32_t index) {
      return const_cast<marker*>(std::as_const(*this).get_marker(index));
   }
   const Furniture::marker* Furniture::get_marker(uint32_t index) const {
      for (auto& marker : this->markers)
         if (marker.index == index)
            return &marker;
      return nullptr;
   }
   Furniture::marker& Furniture::get_or_create_marker(uint32_t index) {
      for (auto& marker : this->markers)
         if (marker.index == index)
            return marker;
      auto& marker = this->markers.emplace_back();
      marker.index = index;
      return marker;
   }

   void Furniture::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      
      if (!intfc.is_winning_record)
         return;
      
      uint32_t last_marker_index = 0; // last seen ENAM
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;

            #pragma region Inherited from ACTI
               #pragma region Components
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
               #pragma endregion
               case 'PNAM':
                  this->marker_color.load(subrecord);
                  break;
               case 'SNAM':
                  subrecord.read(this->looping_sound);
                  intfc.warn_if_ref_is_wrong_type(this->looping_sound, form_type::sound_descriptor, subrecord.signature());
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
            #pragma region FURN
               case 'MNAM':
                  subrecord.read(this->active_markers_and_furn_flags);
                  break;
               case 'WBDT':
                  subrecord.read(this->workbench.type);
                  {
                     int8_t value = -1;
                     subrecord.read(value);
                     if (value == -1)
                        this->workbench.skill = {};
                     else
                        this->workbench.skill = (dovah::skill)(value - dovah::first_skill_actor_value_index);
                  }
                  break;
               case 'NAM1':
                  if (auto& form = this->associated_spell; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
                  break;
               #pragma region Marker structs
                  case 'ENAM':
                     subrecord.read(last_marker_index);
                     break;
                  case 'NAM0':
                     {
                        auto& m = this->get_or_create_marker(last_marker_index);
                        subrecord.read(m.disabled_entry_points);
                     }
                     break;
                  case 'FNMK':
                     {
                        auto& m    = this->get_or_create_marker(last_marker_index);
                        auto& form = m.keyword;
                        if (subrecord.read(form))
                           intfc.warn_if_ref_is_wrong_type(form, form_type::keyword, subrecord.signature());
                     }
                     break;
                  case 'XMRK':
                     subrecord.read(this->marker_model);
                     if (this->marker_model.size() > max_marker_model_path_length) {
                        // TODO: Should we warn on this?
                     }
                     break;
               #pragma endregion
               case 'FNPR':
                  {
                     auto& item = this->marker_nif_infos.emplace_back();
                     subrecord.read(item.supported_animations);
                     subrecord.read(item.supported_entry);
                  }
                  break;
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Furniture::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      struct {
         form_id_t interact_keyword;
         form_id_t sound_loop;
         form_id_t sound_activate;
         form_id_t water_type;
         components::destruction_stage_data::use_info_builder destruction_uib;
      } activator = {
         .destruction_uib{uib}
      };
      struct {
         form_id_t associated_spell;
         std::map<uint32_t, form_id_t> marker_keywords;
      } furniture;

      uint32_t last_marker_index = 0; // last seen ENAM
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region Inherited from ACTI
               case 'VMAD':
                  components::papyrus_attachment_data::generate_use_info(subrecord, uib);
                  break;
               case 'SNAM': // looping sound (e.g. nirnroot bell)
                  subrecord.read(activator.sound_loop);
                  break;
               case 'VNAM': // activation sound
                  subrecord.read(activator.sound_activate);
                  break;
               case 'WNAM': // water type, for water activators
                  subrecord.read(activator.water_type);
                  break;
               case 'KNAM': // interaction keyword
                  subrecord.read(activator.interact_keyword);
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
                  components::destruction_stage_data::generate_use_info(subrecord, activator.destruction_uib);
                  break;
               case 'KSIZ':
               case 'KWDA':
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
            #pragma region FURN
               case 'MNAM':
               case 'WDBT':
                  break;
               case 'NAM1':
                  subrecord.read(furniture.associated_spell);
                  break;
               #pragma region Marker structs
                  case 'ENAM':
                     subrecord.read(last_marker_index);
                     break;
                  case 'NAM0':
                     break;
                  case 'FNMK':
                     {
                        auto& form = furniture.marker_keywords[last_marker_index];
                        subrecord.read(form);
                     }
                     break;
               #pragma endregion
               case 'FNPR':
                  break;
            #pragma endregion
         }
      }
      uib.add_outbound_reference(activator.interact_keyword);
      uib.add_outbound_reference(activator.sound_loop);
      uib.add_outbound_reference(activator.sound_activate);
      uib.add_outbound_reference(activator.water_type, decltype(Furniture::water_type)::use_info_flag);
      activator.destruction_uib.done();

      uib.add_outbound_reference(furniture.associated_spell);
      for (auto& pair : furniture.marker_keywords) {
         uib.add_outbound_reference(pair.second);
      }
   }
   void Furniture::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Furniture*)out;
      
      #pragma region Inherited from ACTI
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
         copy->activation_sound.set(*copy, this->activation_sound);
         copy->water_type.set(*copy, this->water_type);
         copy->interact_keyword.set(*copy, this->interact_keyword);
         copy->activation_verb = this->activation_verb;
         copy->activator_flags = this->activator_flags;
      #pragma endregion
      #pragma region FURN
         copy->active_markers_and_furn_flags = this->active_markers_and_furn_flags;
         copy->workbench = this->workbench;
         copy->associated_spell.set(*copy, this->associated_spell);
         {
            auto& src_list = this->markers;
            auto& dst_list = copy->markers;
            for (auto& item : dst_list) {
               item.keyword.set(*copy, nullptr);
            }
            dst_list.clear();

            size_t size = src_list.size();
            dst_list.resize(size);
            for (size_t i = 0; i < size; ++i) {
               auto& src_item = src_list[i];
               auto& dst_item = dst_list[i];
               dst_item.disabled_entry_points = src_item.disabled_entry_points;
               dst_item.index                 = src_item.index;
               dst_item.keyword.set(*copy, src_item.keyword);
            }
         }
         copy->marker_nif_infos = this->marker_nif_infos;
         copy->marker_model = this->marker_model;
      #pragma endregion
   }
   void Furniture::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      #pragma region FORM
         this->script_data.save(record, intfc);
         auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
         this->bounds.save(OBND, intfc);
         OBND.close();
      #pragma endregion
      #pragma region ACTI
         if (!this->name.empty()) {
            auto& FULL = record.open_next_subrecord('FULL');
            FULL.write(this->name);
            FULL.close();
         }
         this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
         if (this->destruction_data.has_value())
            this->destruction_data.value().save(record, intfc);
         this->keywords.save(record, intfc);

         auto& PNAM = record.open_next_subrecord('PNAM');
         this->marker_color.save(PNAM);
         PNAM.close();
         record.write_formID_subrecord('SNAM', this->looping_sound, true);
         record.write_formID_subrecord('VNAM', this->activation_sound, true);
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
      #pragma endregion
      #pragma region FURN
         {
            auto& subrecord = record.open_next_subrecord('MNAM');
            subrecord.write(this->active_markers_and_furn_flags);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('WBDT');
            subrecord.write(this->workbench.type);
            int8_t skill = -1;
            if (auto& opt = this->workbench.skill; opt.has_value()) {
               skill = (int8_t)opt.value() + dovah::first_skill_actor_value_index;
            }
            subrecord.write(skill);
            subrecord.close();
         }
         record.write_formID_subrecord('NAM1', this->associated_spell, true);
         for (auto& marker : this->markers) {
            bool is_empty = !marker.disabled_entry_points && !marker.keyword;
            if (is_empty)
               continue;

            {
               auto& subrecord = record.open_next_subrecord('ENAM');
               subrecord.write(marker.index);
               subrecord.close();
            }
            if (marker.disabled_entry_points != 0) {
               auto& subrecord = record.open_next_subrecord('NAM0');
               subrecord.write(marker.disabled_entry_points);
               subrecord.close();
            }
            record.write_formID_subrecord('FNMK', marker.keyword, true);
         }
         for (auto& info : this->marker_nif_infos) {
            auto& subrecord = record.open_next_subrecord('FNPR');
            subrecord.write(info.supported_animations);
            subrecord.write(info.supported_entry);
            subrecord.close();
         }
         if (!this->marker_model.empty()) {
            record.write_string_subrecord('XMRK', this->marker_model);
         }
      #pragma endregion
   }
   void Furniture::_sever_outbound_references_impl(form_stub& other) noexcept {
      #pragma region Inherited from ACTI
         this->script_data.sever_outbound_references_to(other, *this);
         this->model.sever_outbound_references_to(other, *this);
         if (this->destruction_data.has_value())
            this->destruction_data.value().sever_outbound_references_to(other, *this);
         this->keywords.sever_outbound_references_to(other, *this);
         
         this->looping_sound.clear_if(*this, other);
         this->activation_sound.clear_if(*this, other);
         this->water_type.clear_if(*this, other);
         this->interact_keyword.clear_if(*this, other);
      #pragma endregion
      #pragma region FURN
         this->associated_spell.clear_if(*this, other);
         for (auto& marker : this->markers)
            marker.keyword.clear_if(*this, other);
      #pragma endregion
   }
   void Furniture::_clear_impl() noexcept {
      #pragma region Inherited from ACTI
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
         this->activation_sound.set(*this, nullptr);
         this->water_type.set(*this, nullptr);
         this->interact_keyword.set(*this, nullptr);
         this->activation_verb.reset();
         this->activator_flags = 0;
      #pragma endregion
      #pragma region FURN
         this->active_markers_and_furn_flags = 0;
         this->workbench = {};
         this->associated_spell.set(*this, nullptr);
         for (auto& marker : this->markers)
            marker.keyword.set(*this, nullptr);
         this->markers.clear();
         this->marker_nif_infos.clear();
         this->marker_model.clear();
      #pragma endregion
   }
}