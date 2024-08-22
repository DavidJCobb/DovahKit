#include "Race.h"
#include "_common_cpp.h"

#include "../data/hardcoded_form_ids.h"

#include "../notices/form_load_warnings/by_form_type/race/biped_object_name_too_long.h"
#include "../notices/form_load_warnings/by_form_type/race/invalid_boosted_skill.h"
#include "../notices/form_load_warnings/by_form_type/race/invalid_face_texture_sex.h"
#include "../notices/form_load_warnings/by_form_type/race/invalid_morph_bitmask_index.h"
#include "../notices/form_load_warnings/by_form_type/race/tint_layer_data_before_tint_layer.h"
#include "../notices/form_load_warnings/by_form_type/race/too_many_biped_object_names.h"
#include "../notices/form_load_warnings/by_form_type/race/too_many_phonemes.h"
#include "../notices/form_load_warnings/by_form_type/race/wrong_weight_count_per_phoneme.h"
//
#include "../notices/form_save_errors/by_form_type/race/biped_object_name_is_too_long.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::race;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::race;
   }
}

namespace dovah::loaded_forms {
   void Race::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      
      bool is_female    = false; // FNAM enables; MNAM disables
      bool in_head_data = false; // NAM0
      bool in_body_data = false; // NAM1

      size_t current_biped_object_name  = 0;
      size_t current_phoneme_weight_set = 0;

      // The fields in `tint_preset` are loaded individually, rather than each subrecord 
      // applying to the last-loaded preset.
      //
      // That is: the following two sequences of subrecords are equivalent:
      // 
      //    TINV TINC TIRS TINV TINC TIRS
      //    TINV TINV TINC TINC TIRS TIRS
      //
      size_t current_tint_preset_alpha = 0; // TINV
      size_t current_tint_preset_color = 0; // TINC
      size_t current_tint_preset_index = 0; // TIRS

      uint32_t tri_morph_flags_index = 0;

      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            #pragma region Components
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case components::attack_data::subrecord_signature_race:
            case components::attack_data::subrecord_signature_data:
               this->attack_data.load(record, intfc);
               break;
            case components::biped_object::subrecord_signature_deprecated:
            case components::biped_object::subrecord_signature_modern:
               this->biped_object.load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case components::spell_list::subrecord_signature_count:
            case components::spell_list::subrecord_signature_entry:
               this->spells.load(subrecord, intfc);
               break;
            #pragma endregion
            #pragma region Components only in Bethesda&apos;s code
            case 'WNAM': // BGSSkinForm (signature varies per-form)
               if (subrecord.read(this->skin)) {
                  intfc.warn_if_ref_is_wrong_type(this->skin, form_type::armor, subrecord.signature());
               }
               break;
            #pragma endregion

            case 'DATA':
               {
                  for (size_t i = 0; i < this->stats.skill_boosts.size(); ++i) {
                     auto& dst_opt = this->stats.skill_boosts[i];

                     int8_t  skill_index = -1;
                     uint8_t skill_boost =  0;

                     subrecord.read(skill_index);
                     subrecord.read(skill_boost);
                     if (skill_index != -1) {
                        skill_index -= first_skill_actor_value_index;
                        if (skill_index >= skill_count) {
                           specific_load_warnings::invalid_boosted_skill notice(
                              this->stub,
                              skill_index,
                              i
                           );
                           intfc.log_load_warning(notice);
                        }
                     }

                     if (skill_index == -1) {
                        dst_opt = {};
                     } else {
                        auto& dst = dst_opt.emplace();
                        dst.skill = (skill)skill_index;
                        dst.boost = skill_boost;
                     }
                  }
                  subrecord.skip_bytes(2);
                  for (size_t sex = 0; sex < sex_count; ++sex)
                     subrecord.read(this->by_sex[sex].height_mult);
                  for (size_t sex = 0; sex < sex_count; ++sex)
                     subrecord.read(this->by_sex[sex].weight);
                  subrecord.read(this->race_flags);
                  subrecord.read(this->stats.attribute_base.health);
                  subrecord.read(this->stats.attribute_base.magicka);
                  subrecord.read(this->stats.attribute_base.stamina);
                  subrecord.read(this->stats.base_carry_capacity);
                  subrecord.read(this->stats.base_mass);
                  subrecord.read(this->movement.acceleration_rate);
                  subrecord.read(this->movement.deceleration_rate);
                  subrecord.read(this->stats.creature_size);
                  subrecord.read(this->biped_object_info.head);
                  subrecord.read(this->biped_object_info.hair);
                  subrecord.read(this->stats.injured_health_threshold);
                  subrecord.read(this->biped_object_info.shield);
                  subrecord.read(this->stats.attribute_regen.health);
                  subrecord.read(this->stats.attribute_regen.magicka);
                  subrecord.read(this->stats.attribute_regen.stamina);
                  subrecord.read(this->stats.unarmed.damage);
                  subrecord.read(this->stats.unarmed.reach);
                  subrecord.read(this->biped_object_info.body);
                  subrecord.read(this->stats.aim_angle_tolerance);
                  subrecord.read(this->movement.angular_tolerance);
                  subrecord.read(this->alt_flags);
                  if (record.version() >= 43) {
                     subrecord.read(this->mount_data.climb_on_offset.x);
                     subrecord.read(this->mount_data.climb_on_offset.y);
                     subrecord.read(this->mount_data.climb_on_offset.z);
                     subrecord.read(this->mount_data.dismount_offset.x);
                     subrecord.read(this->mount_data.dismount_offset.y);
                     subrecord.read(this->mount_data.dismount_offset.z);
                     subrecord.read(this->mount_data.camera_offset.x);
                     subrecord.read(this->mount_data.camera_offset.y);
                     subrecord.read(this->mount_data.camera_offset.z);
                  }
               }
               break;

            #pragma region Single forms
               #pragma region Movement types
                  case 'WKMV':
                     if (auto& dst = this->movement.types.walk; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
                  case 'RNMV':
                     if (auto& dst = this->movement.types.run; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
                  case 'SWMV':
                     if (auto& dst = this->movement.types.swim; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
                  case 'FLMV':
                     if (auto& dst = this->movement.types.fly; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
                  case 'SNMV':
                     if (auto& dst = this->movement.types.sneak; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
                  case 'SPMV':
                     if (auto& dst = this->movement.types.sprint; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::movement_type, subrecord.signature());
                     }
                     break;
               #pragma endregion
               case 'NAM4':
                  if (subrecord.read(this->material_type)) {
                     intfc.warn_if_ref_is_wrong_type(this->material_type, form_type::material_type, subrecord.signature());
                  }
                  break;
               case 'NAM5':
                  if (subrecord.read(this->impact_data_set)) {
                     intfc.warn_if_ref_is_wrong_type(this->impact_data_set, form_type::impact_data_set, subrecord.signature());
                  }
                  break;
               case 'NAM7':
                  if (subrecord.read(this->decapitation_effect)) {
                     intfc.warn_if_ref_is_wrong_type(this->decapitation_effect, form_type::art_object, subrecord.signature());
                  }
                  break;
               case 'NAM8':
                  if (subrecord.read(this->morph_race)) {
                     intfc.warn_if_ref_is_wrong_type(this->morph_race, form_type::race, subrecord.signature());
                  }
                  break;
               //
               case 'GNAM':
                  if (subrecord.read(this->body_part_data)) {
                     intfc.warn_if_ref_is_wrong_type(this->body_part_data, form_type::body_part_data, subrecord.signature());
                  }
                  break;
               case 'LNAM':
                  if (auto& dst = this->container_sounds.close; subrecord.read(dst)) {
                     intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
                  }
                  break;
               case 'ONAM':
                  if (auto& dst = this->container_sounds.open; subrecord.read(dst)) {
                     intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
                  }
                  break;
               case 'PNAM':
                  subrecord.read(this->facegen_clamp.main);
                  break;
               case 'QNAM':
                  if (subrecord.read(form_id)) {
                     intfc.warn_if_ref_is_wrong_type(form_id, form_type::equip_slot, subrecord.signature());
                     this->equipment.equip_slots.push_back(form_id);
                  }
                  break;
               case 'RNAM':
                  if (subrecord.read(this->armor_race)) {
                     intfc.warn_if_ref_is_wrong_type(this->armor_race, form_type::race, subrecord.signature());
                  }
                  break;
               case 'UNES':
                  if (auto& dst = this->equipment.unarmed_equip_slot; subrecord.read(dst)) {
                     intfc.warn_if_ref_is_wrong_type(dst, form_type::equip_slot, subrecord.signature());
                  }
                  break;
            #pragma endregion

            case 'MTNM':
               //
               // In current versions of Skyrim, these are always the same FourCCs. The game 
               // itself doesn't even load them, but the CK always generates them. We'll check 
               // for the case here so we don't emit a warning on an unrecognized subrecord, 
               // but we'll ignore it. (There aren't any form IDs, etc., inside, so that's 
               // safe to do.)
               //
               break;
            case 'NAME':
               if (current_biped_object_name < this->biped_object_info.names.size()) {
                  auto& name = this->biped_object_info.names[current_biped_object_name];
                  if (subrecord.read(name)) {
                     if (name.size() > max_biped_object_name_length) {
                        specific_load_warnings::biped_object_name_too_long notice(
                           this->stub,
                           name.size(),
                           current_biped_object_name
                        );
                        intfc.log_load_warning(notice);
                     }
                  }
               }
               ++current_biped_object_name;
               break;
            case 'UNAM':
               subrecord.read(this->facegen_clamp.face);
               break;
            case 'VNAM':
               subrecord.read(this->equipment.flags);
               break;

            // "Marker" subrecords that set loader state:
            case 'FNAM':
               is_female = true;
               break;
            case 'MNAM':
               is_female = false;
               break;
               //
            case 'NAM0':
               in_body_data = false;
               in_head_data = true;
               break;
            case 'NAM1':
               in_body_data = true;
               in_head_data = false;
               break;
            case 'NAM2':
               //
               // Removed in a game update. No code remains in TESV.exe or CreationKit.exe.
               //
               break;
            case 'NAM3':
            case 'NAM6':
               in_body_data = false;
               in_head_data = false;
               break;

            #pragma region Sex-differentiated data (except face tints)
               #pragma region Subrecords that specify a sex
                  case 'XNAM': // Alternative to FSTF and FSTM, with each XNAM specifying a sex rather than relying on prior "marker" subrecords
                     {
                        uint32_t         unused;
                        form_reference_t texture = {};
                        uint32_t         sex = 0;
                        if (subrecord.read(unused) && subrecord.read(texture) && subrecord.read(sex)) {
                           if (sex < 2) {
                              intfc.warn_if_ref_is_wrong_type(texture, form_type::texture_set, subrecord.signature());
                              (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_textures.push_back(texture);
                           } else {
                              specific_load_warnings::invalid_face_texture_sex notice(
                                 this->stub,
                                 sex,
                                 texture.get_form_stub()
                              );
                              intfc.log_load_warning(notice);
                           }
                        }
                     }
                     break;
               #pragma endregion
               #pragma region Stateful subrecords (relying on loader state to know what sex to load into)
                  case 'ANAM':
                     (is_female ? this->by_sex.female : this->by_sex.male).skeleton_nif.load_model_path(subrecord, intfc);
                     break;
                  case 'INDX':
                     //
                     // INDX is the index of each HeadPart in the list that it came from... but it's not 
                     // actually used anywhere. The loader reads it but never uses the variable it reads 
                     // into; and the CK's save code just writes, for each HeadPart, its index in the 
                     // HeadPart list. The value is completely unused.
                     // 
                     // My guess is that Bethesda anticipated a potential need to refer to HeadParts via 
                     // a consistent index/ID, similar to tint layers, but it never came to pass -- and 
                     // they never bothered to remove the subrecord. In any case, we should generate it 
                     // on save, but since the game never retains it in memory, neither will we.
                     //
                     break;
                  case 'HEAD':
                     if (!in_head_data)
                        break;
                     if (subrecord.read(form_id)) {
                        intfc.warn_if_ref_is_wrong_type(form_id, form_type::head_part, subrecord.signature());
                        (is_female ? this->by_sex.female : this->by_sex.male).head_data.head_parts.push_back(form_id);
                     }
                     break;
               #pragma endregion
               #pragma region Sex-paired subrecords (specifying data for both sexes at once)
                  case 'DNAM':
                     if (subrecord.size() != 8)
                        break;
                     if (auto& dst = this->by_sex.male.decapitate_armor; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::armor, subrecord.signature());
                     }
                     if (auto& dst = this->by_sex.female.decapitate_armor; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::armor, subrecord.signature());
                     }
                     break;
                  case 'HCLF':
                     if (subrecord.size() != 8)
                        break;
                     if (auto& dst = this->by_sex.male.head_data.default_hair_color; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::color, subrecord.signature());
                     }
                     if (auto& dst = this->by_sex.female.head_data.default_hair_color; subrecord.read(dst)) {
                        intfc.warn_if_ref_is_wrong_type(dst, form_type::color, subrecord.signature());
                     }
                     break;
                  case 'INAM': // Alternative to MNAM~HEAD+FNAM~HEAD, loading one HeadPart for each sex.
                  case 'JNAM':
                     if (subrecord.read(form_id)) {
                        auto& dst = this->by_sex.male.head_data.head_parts;
                        intfc.warn_if_ref_is_wrong_type(form_id, form_type::head_part, subrecord.signature());
                        dst.push_back(form_id);
                     }
                     if (subrecord.read(form_id)) {
                        auto& dst = this->by_sex.female.head_data.head_parts;
                        intfc.warn_if_ref_is_wrong_type(form_id, form_type::head_part, subrecord.signature());
                        dst.push_back(form_id);
                     }
                     break;
                  case 'VTCK':
                     switch (subrecord.size()) {
                        case 4:
                           this->by_sex.female.voicetype.unmanaged_set(subrecord.lookup_form_by_id(hardcoded_form_ids::AdultFemaleVoice1));
                           this->by_sex.male.voicetype.unmanaged_set(subrecord.lookup_form_by_id(hardcoded_form_ids::AdultMaleVoice1));
                           break;
                        case 8:
                           if (auto& dst = this->by_sex.male.voicetype; subrecord.read(dst)) {
                              intfc.warn_if_ref_is_wrong_type(dst, form_type::voicetype, subrecord.signature());
                              if (dst == nullptr) {
                                 dst.unmanaged_set(subrecord.lookup_form_by_id(hardcoded_form_ids::AdultMaleVoice1));
                              }
                           }
                           if (auto& dst = this->by_sex.female.voicetype; subrecord.read(dst)) {
                              intfc.warn_if_ref_is_wrong_type(dst, form_type::voicetype, subrecord.signature());
                              if (dst == nullptr) {
                                 dst.unmanaged_set(subrecord.lookup_form_by_id(hardcoded_form_ids::AdultFemaleVoice1));
                              }
                           }
                           break;
                     }
                     break;
               #pragma endregion
               #pragma region Sex-specific subrecords
                  #pragma region Available Hair Color
                     case 'AHCF':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::color, subrecord.signature());
                           this->by_sex.female.head_data.hair_colors.push_back(form_id);
                        }
                        break;
                     case 'AHCM':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::color, subrecord.signature());
                           this->by_sex.male.head_data.hair_colors.push_back(form_id);
                        }
                        break;
                  #pragma endregion
                  #pragma region Default Face TextureSet
                     case 'DFTF':
                        if (auto& dst = this->by_sex.female.head_data.default_face_texture; subrecord.read(dst)) {
                           intfc.warn_if_ref_is_wrong_type(dst, form_type::texture_set, subrecord.signature());
                        }
                        break;
                     case 'DFTM':
                        if (auto& dst = this->by_sex.male.head_data.default_face_texture; subrecord.read(dst)) {
                           intfc.warn_if_ref_is_wrong_type(dst, form_type::texture_set, subrecord.signature());
                        }
                        break;
                  #pragma endregion
                  #pragma region Face TextureSet
                     case 'FTSF':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::texture_set, subrecord.signature());
                           this->by_sex.female.head_data.face_textures.push_back(form_id);
                        }
                        break;
                     case 'FTSM':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::texture_set, subrecord.signature());
                           this->by_sex.male.head_data.face_textures.push_back(form_id);
                        }
                        break;
                  #pragma endregion
                  #pragma region Race Preset
                     case 'RPRF':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::actor_base, subrecord.signature());
                           this->by_sex.female.head_data.preset_actors.push_back(form_id);
                        }
                        break;
                     case 'RPRM':
                        if (subrecord.read(form_id)) {
                           intfc.warn_if_ref_is_wrong_type(form_id, form_type::actor_base, subrecord.signature());
                           this->by_sex.male.head_data.preset_actors.push_back(form_id);
                        }
                        break;
                  #pragma endregion
               #pragma endregion
            #pragma endregion
            #pragma region Tint data
               case 'TINL':
                  subrecord.read(this->tint_layer_count);
                  break;
               #pragma region Tint layers (sex-differentiated, relying on loader state)
                  case 'TINI':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        auto& item = list.emplace_back();
                        subrecord.read(item.index);
                        //
                        current_tint_preset_alpha = 0; // index of the preset that the next TINV will write to
                        current_tint_preset_color = 0; // index of the preset that the next TINC will write to
                        current_tint_preset_index = 0; // index of the preset that the next TIRS will write to
                     }
                     break;
                  case 'TINT':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& item = list.back();
                        subrecord.read(item.texture);
                     }
                     break;
                  case 'TINP':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& item = list.back();
                        subrecord.read(item.type);
                     }
                     break;
                  case 'TIND':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& item = list.back();
                        subrecord.read(item.default_color);
                     }
                     break;
                  case 'TIRS':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& tint = list.back();
                        {
                           auto i = current_tint_preset_index;
                           if (i >= tint.presets.size()) {
                              tint.presets.resize(i + 1);
                           }
                           auto& preset = tint.presets[i];
                           subrecord.read(preset.index);
                        }
                        ++current_tint_preset_index;
                     }
                     break;
                  case 'TINC':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& tint = list.back();
                        {
                           auto i = current_tint_preset_color;
                           if (i >= tint.presets.size()) {
                              tint.presets.resize(i + 1);
                           }
                           auto& preset = tint.presets[i];
                           if (subrecord.read(preset.color)) {
                              intfc.warn_if_ref_is_wrong_type(preset.color, form_type::color, subrecord.signature());
                           }
                        }
                        ++current_tint_preset_color;
                     }
                     break;
                  case 'TINV':
                     {
                        auto& list = (is_female ? this->by_sex.female : this->by_sex.male).head_data.face_tints;
                        if (list.empty()) {
                           specific_load_warnings::tint_layer_data_before_tint_layer notice(
                              this->stub,
                              subrecord.signature()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                        auto& tint = list.back();
                        {
                           auto i = current_tint_preset_alpha;
                           if (i >= tint.presets.size()) {
                              tint.presets.resize(i + 1);
                           }
                           auto& preset = tint.presets[i];
                           subrecord.read(preset.alpha);
                        }
                        ++current_tint_preset_alpha;
                     }
                     break;
               #pragma endregion
            #pragma endregion
            #pragma region Head morph flags (sex-differentiated, relying on loader state)
            case 'MPAI':
               if (subrecord.read(tri_morph_flags_index)) {
                  if (tri_morph_flags_index >= 4) {
                     specific_load_warnings::invalid_morph_bitmask_index notice(
                        this->stub,
                        tri_morph_flags_index
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'MPAV':
               if (tri_morph_flags_index < 4) {
                  auto& morphs = (is_female ? this->by_sex.female : this->by_sex.male).head_data.morphs;

                  cobb::bitset<256>* dst = nullptr;
                  switch (tri_morph_flags_index) {
                     case 0:
                        dst = &morphs.noses;
                        break;
                     case 1:
                        dst = &morphs.brows;
                        break;
                     case 2:
                        dst = &morphs.eyes;
                        break;
                     case 3:
                        dst = &morphs.mouths;
                        break;
                     default:
                        std::unreachable();
                  }

                  {
                     using span_type = uint32_t;
                     constexpr const size_t span_bitcount = sizeof(span_type) * 8;
                     constexpr const size_t span_count    = 256 / span_bitcount;

                     for (size_t i = 0; i < span_count; ++i) {
                        span_type bits = 0xFFFFFFFF;
                        subrecord.read(bits);

                        dst->overwrite_span(i * span_bitcount, bits);
                     }
                  }
               }
               break;
            #pragma endregion

            #pragma region Phoneme morphs
               case 'PHTN':
                  {
                     auto& item = this->phonemes.morph_names.emplace_back();
                     subrecord.read(item);
                  }
                  break;
               case 'PHKV':
                  //
                  // No-op.
                  //
                  break;
               case 'PHWT':
                  if (current_phoneme_weight_set < this->phonemes.weights.size()) {
                     auto& list = this->phonemes.weights[current_phoneme_weight_set];

                     size_t count = subrecord.size() / sizeof(float);
                     if (count != this->phonemes.morph_names.size()) {
                        bool uses_default_names = (
                           count == 16
                           &&
                           (this->race_flags & race_flag::facegen_head)
                           &&
                           this->phonemes.morph_names.empty()
                        );
                        if (!uses_default_names) {
                           specific_load_warnings::wrong_weight_count_per_phoneme notice(
                              this->stub,
                              (dovah::face_fx::phoneme)current_phoneme_weight_set,
                              count,
                              this->phonemes.morph_names.size()
                           );
                           intfc.log_load_warning(notice);
                        }
                     }
                     list.clear();
                     list.resize(count);
                     for (size_t i = 0; i < count; ++i) {
                        subrecord.read(list[i]);
                     }
                  }
                  ++current_phoneme_weight_set;
                  break;
            #pragma endregion

            case 'ENAM':
            case 'HNAM':
            case 'KNAM':
            case 'NNAM':
            case 'SNAM':
            case 'TNAM':
               //
               // The above are more likely a `default` case in Bethesda's code rather than 
               // actually being intended to contribute to model loading.
               //
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               if (in_body_data) {
                  (is_female ? this->by_sex.female : this->by_sex.male).lighting_model.load(subrecord, intfc);
                  break;
               } else {
                  (is_female ? this->by_sex.female : this->by_sex.male).behavior_graph.load(subrecord, intfc);
                  break;
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (record.version() < 42) {
         if (this->race_flags & race_flag::playable) {
            if (this->race_flags & race_flag::can_dual_wield)
               this->equipment.flags |=  equipment_flag::crossbow;
            else
               this->equipment.flags &= ~equipment_flag::crossbow;
         }
      }

      if (current_biped_object_name > this->biped_object_info.names.size()) {
         specific_load_warnings::too_many_biped_object_names notice(
            this->stub,
            current_biped_object_name
         );
         intfc.log_load_warning(notice);
      }
      if (current_phoneme_weight_set > this->phonemes.weights.size()) {
         specific_load_warnings::too_many_phonemes notice(
            this->stub,
            current_phoneme_weight_set
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void Race::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      struct sex_single_uses {
         form_id_t decapitation_armor = 0; // DNAM
         form_id_t default_face_texture_set = 0;
         form_id_t default_hair_color = 0;
         form_id_t voicetype = 0; // VTCK and others
      };

      components::attack_data::use_info_state attack_data;

      form_id_t armor_race = 0; // RNAM
      form_id_t body_part_data = 0; // GNAM
      form_id_t decapitation_effect = 0; // NAM7
      form_id_t impact_data_set = 0; // NAM5
      form_id_t material_type = 0; // NAM4
      form_id_t morph_race = 0; // NAM8
      form_id_t unarmed_equip_slot = 0;   // UNES
      form_id_t movement_type_fly = 0;    // FLMV
      form_id_t movement_type_run = 0;    // RNMV
      form_id_t movement_type_sneak = 0;  // SNMV
      form_id_t movement_type_sprint = 0; // SPMV
      form_id_t movement_type_swim = 0;   // SWMV
      form_id_t movement_type_walk = 0;   // WKMV
      form_id_t skin = 0; // WNAM
      form_id_t sound_open = 0; // ONAM
      form_id_t sound_close = 0; // LNAM

      sex_single_uses female_forms;
      sex_single_uses male_forms;

      bool female          = false;
      bool in_body_data    = false;
      bool in_head_data    = false;
      bool any_tint_loaded = false;

      form_id_t current_tint_default_color = 0;

      form_id_t form_id = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
            case 'DESC':
            case 'DATA':
            case 'MPAI':
            case 'MPAV':
            case 'NAME':
            case 'PHKV':
            case 'PHTN':
            case 'PHWT':
            case 'TINL':
            case 'TINP':
            case 'TINT':
            case 'TINV':
            case 'TIRS':
            case 'ANAM':
            case 'PNAM':
            case 'UNAM':
            case 'VNAM':
            case components::biped_object::subrecord_signature_deprecated:
            case components::biped_object::subrecord_signature_modern:
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               break;
            case components::attack_data::subrecord_signature_race:
            case components::attack_data::subrecord_signature_data:
            case components::attack_data::subrecord_signature_event:
               attack_data.read(record);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case components::spell_list::subrecord_signature_count:
            case components::spell_list::subrecord_signature_entry:
               components::spell_list::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'FNAM':
               female = true;
               break;
            case 'MNAM':
               female = false;
               break;
            case 'NAM0':
               in_head_data = true;
               in_body_data = false;
               break;
            case 'NAM1':
               in_head_data = false;
               in_body_data = true;
               break;
            case 'NAM3':
            case 'NAM6':
               in_head_data = false;
               in_body_data = false;
               break;
               
            case 'TINI':
               if (any_tint_loaded) {
                  uib.add_outbound_reference(current_tint_default_color);
                  current_tint_default_color = 0;
               }
               any_tint_loaded = true;
               break;
            case 'TIND':
               if (!any_tint_loaded)
                  break;
               subrecord.read(current_tint_default_color);
               break;

            case 'RNAM':
               subrecord.read(armor_race);
               break;
            case 'GNAM':
               subrecord.read(body_part_data);
               break;
            case 'NAM7':
               subrecord.read(decapitation_effect);
               break;
            case 'NAM5':
               subrecord.read(impact_data_set);
               break;
            case 'NAM4':
               subrecord.read(material_type);
               break;
            case 'NAM8':
               subrecord.read(morph_race);
               break;
            case 'FLMV':
               subrecord.read(movement_type_fly);
               break;
            case 'RNMV':
               subrecord.read(movement_type_run);
               break;
            case 'SNMV':
               subrecord.read(movement_type_sneak);
               break;
            case 'SPMV':
               subrecord.read(movement_type_sprint);
               break;
            case 'SWMV':
               subrecord.read(movement_type_swim);
               break;
            case 'WKMV':
               subrecord.read(movement_type_walk);
               break;
            case 'WNAM':
               subrecord.read(skin);
               break;
            case 'ONAM':
               subrecord.read(sound_open);
               break;
            case 'LNAM':
               subrecord.read(sound_close);
               break;
            case 'UNES':
               subrecord.read(unarmed_equip_slot);
               break;

            // Single-form subrecords that feed into lists:
            case 'QNAM':
            case 'AHCF':
            case 'AHCM':
            case 'FSTF':
            case 'FSTM':
            case 'RPRF':
            case 'RPRM':
               if (subrecord.read(form_id) && form_id)
                  uib.add_outbound_reference(form_id);
               break;

            case 'DFTF':
               subrecord.read(female_forms.default_face_texture_set);
               break;
            case 'DFTM':
               subrecord.read(male_forms.default_face_texture_set);
               break;
            case 'DNAM':
               subrecord.read(male_forms.decapitation_armor);
               subrecord.read(female_forms.decapitation_armor);
               break;
            case 'HCLF':
               subrecord.read(male_forms.default_hair_color);
               subrecord.read(female_forms.default_hair_color);
               break;
            case 'HEAD':
               if (in_head_data) {
                  form_id_t headpart;
                  if (subrecord.read(headpart) && headpart)
                     uib.add_outbound_reference(headpart);
               }
               break;
            case 'INAM':
            case 'JNAM':
               {
                  form_id_t headpart;
                  if (subrecord.read(headpart) && headpart) // male
                     uib.add_outbound_reference(headpart);
                  if (subrecord.read(headpart) && headpart) // female
                     uib.add_outbound_reference(headpart);
               }
               break;
            case 'VTCK':
               switch (subrecord.size()) {
                  case 4:
                     male_forms.voicetype   = 0;
                     female_forms.voicetype = 0;
                     break;
                  case 8:
                     subrecord.read(male_forms.voicetype);
                     subrecord.read(female_forms.voicetype);
                     break;
               }
               break;
            case 'XNAM':
               {
                  uint32_t  unused;
                  form_id_t form;
                  uint32_t  sex = 0;
                  if (subrecord.read(unused) && subrecord.read(form) && subrecord.read(sex)) {
                     if (sex < sex_count) {
                        uib.add_outbound_reference(form);
                     }
                  }
               }
               break;
         }
      }

      if (!male_forms.voicetype)
         male_forms.voicetype = hardcoded_form_ids::AdultMaleVoice1;
      if (!female_forms.voicetype)
         female_forms.voicetype = hardcoded_form_ids::AdultFemaleVoice1;

      if (any_tint_loaded && current_tint_default_color)
         uib.add_outbound_reference(current_tint_default_color);

      attack_data.commit(uib);
      uib.add_outbound_reference(armor_race);
      uib.add_outbound_reference(body_part_data);
      uib.add_outbound_reference(decapitation_effect);
      uib.add_outbound_reference(impact_data_set);
      uib.add_outbound_reference(material_type);
      uib.add_outbound_reference(morph_race);
      uib.add_outbound_reference(unarmed_equip_slot);
      uib.add_outbound_reference(movement_type_fly);
      uib.add_outbound_reference(movement_type_run);
      uib.add_outbound_reference(movement_type_sneak);
      uib.add_outbound_reference(movement_type_sprint);
      uib.add_outbound_reference(movement_type_swim);
      uib.add_outbound_reference(movement_type_walk);
      uib.add_outbound_reference(skin);
      uib.add_outbound_reference(sound_open);
      uib.add_outbound_reference(sound_close);
      {
         auto& sex = female_forms;
         uib.add_outbound_reference(sex.decapitation_armor);
         uib.add_outbound_reference(sex.default_face_texture_set);
         uib.add_outbound_reference(sex.default_hair_color);
         uib.add_outbound_reference(sex.voicetype);
      }
      {
         auto& sex = male_forms;
         uib.add_outbound_reference(sex.decapitation_armor);
         uib.add_outbound_reference(sex.default_face_texture_set);
         uib.add_outbound_reference(sex.default_hair_color);
         uib.add_outbound_reference(sex.voicetype);
      }
   }
   void Race::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Race*)out;

      copy->attack_data.clone_from(this->attack_data, *copy);
      copy->biped_object.clone_from(this->biped_object, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      copy->spells.clone_from(this->spells, *copy);

      copy->name = this->name;
      copy->description = this->description;

      copy->race_flags = this->race_flags;
      copy->alt_flags  = this->alt_flags;
      copy->armor_race.set(*copy, this->armor_race);
      copy->body_part_data.set(*copy, this->body_part_data);
      copy->decapitation_effect.set(*copy, this->decapitation_effect);
      copy->impact_data_set.set(*copy, this->impact_data_set);
      copy->material_type.set(*copy, this->material_type);
      copy->morph_race.set(*copy, this->morph_race);
      copy->skin.set(*copy, this->skin);

      copy->tint_layer_count = this->tint_layer_count;
      copy->biped_object_info = this->biped_object_info;

      for (size_t i = 0; i < this->by_sex.size(); ++i) {
         auto& src = this->by_sex[i];
         auto& dst = copy->by_sex[i];

         dst.behavior_graph.clone_from(src.behavior_graph);
         dst.decapitate_armor.set(*copy, src.decapitate_armor);
         {
            auto& src = this->by_sex[i].head_data;
            auto& dst = copy->by_sex[i].head_data;
            dst.default_face_texture.set(*copy, src.default_face_texture);
            dst.default_hair_color.set(*copy, src.default_hair_color);
            copy_form_reference_list(*copy, dst.face_textures, src.face_textures);

            for (auto& item : dst.face_tints) {
               item.default_color.set(*copy, nullptr);
               for (auto& preset : item.presets)
                  preset.color.set(*copy, nullptr);
            }
            dst.face_tints.clear();

            size_t size = src.face_tints.size();
            dst.face_tints.resize(size);
            for (size_t i = 0; i < size; ++i) {
               auto& src_tint = src.face_tints[i];
               auto& dst_tint = dst.face_tints[i];
               dst_tint.index   = src_tint.index;
               dst_tint.texture = src_tint.texture;
               dst_tint.type    = src_tint.type;
               dst_tint.default_color.set(*copy, src_tint.default_color);

               size_t preset_count = src_tint.presets.size();
               dst_tint.presets.resize(preset_count);
               for (size_t j = 0; j < preset_count; ++j) {
                  auto& src_preset = src_tint.presets[j];
                  auto& dst_preset = dst_tint.presets[j];
                  dst_preset.alpha = src_preset.alpha;
                  dst_preset.index = src_preset.index;
                  dst_preset.color.set(*copy, src_preset.color);
               }
            }

            copy_form_reference_list(*copy, dst.hair_colors, src.hair_colors);
            copy_form_reference_list(*copy, dst.head_parts, src.head_parts);
            dst.morphs = src.morphs;
            copy_form_reference_list(*copy, dst.preset_actors, src.preset_actors);
         }
         dst.lighting_model.clone_from(src.lighting_model);
         dst.skeleton_nif.clone_from(src.skeleton_nif);
         dst.voicetype.set(*copy, src.voicetype);
         dst.weight = src.weight;
      }

      copy->container_sounds.open.set(*copy, this->container_sounds.open);
      copy->container_sounds.close.set(*copy, this->container_sounds.close);

      copy->equipment.flags = this->equipment.flags;
      copy_form_reference_list(*copy, copy->equipment.equip_slots, this->equipment.equip_slots);
      copy->equipment.unarmed_equip_slot.set(*copy, this->equipment.unarmed_equip_slot);

      copy->facegen_clamp = this->facegen_clamp;

      {
         auto& src = this->movement;
         auto& dst = copy->movement;
         dst.acceleration_rate = src.acceleration_rate;
         dst.deceleration_rate = src.deceleration_rate;
         dst.angular_acceleration_rate = src.angular_acceleration_rate;
         dst.angular_tolerance = src.angular_tolerance;

         for (size_t i = 0; i < src.types.list.size(); ++i) {
            dst.types.list[i].set(*copy, src.types.list[i]);
         }

         for (auto& item : dst.overrides) {
            item.type.set(*copy, nullptr);
         }
         dst.overrides.clear();
         //
         size_t size = src.overrides.size();
         dst.overrides.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst.overrides[i].type.set(*copy, src.overrides[i].type);
            dst.overrides[i].values = src.overrides[i].values;
         }
      }

      copy->phonemes = this->phonemes;
      copy->stats = this->stats;
      copy->mount_data = this->mount_data;
   }
   void Race::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();

      auto& DESC = record.open_next_subrecord('FULL');
      DESC.write(this->description);
      DESC.close();

      this->spells.save(record, intfc);
      record.write_formID_subrecord('WNAM', this->skin, true);
      this->biped_object.save(record, intfc);
      this->keywords.save(record, intfc);
      {
         auto& DATA = record.open_next_subrecord('DATA');
         for (auto& dst_opt : this->stats.skill_boosts) {
            if (!dst_opt.has_value()) {
               DATA.write((int8_t)-1);
               DATA.write((uint8_t)0);
               continue;
            }
            auto& dst = dst_opt.value();
            int8_t skill_index = (int8_t)dst.skill;
            if (skill_index >= 0) {
               skill_index += first_skill_actor_value_index;
            } else {
               skill_index = -1;
            }
            DATA.write(skill_index);
            DATA.write(dst.boost);
         }
         DATA.skip_bytes(2);
         DATA.write(this->by_sex.male.height_mult);
         DATA.write(this->by_sex.female.height_mult);
         DATA.write(this->by_sex.male.weight);
         DATA.write(this->by_sex.female.weight);
         DATA.write(this->race_flags);
         DATA.write(this->stats.attribute_base.health);
         DATA.write(this->stats.attribute_base.magicka);
         DATA.write(this->stats.attribute_base.stamina);
         DATA.write(this->stats.base_carry_capacity);
         DATA.write(this->stats.base_mass);
         DATA.write(this->movement.acceleration_rate);
         DATA.write(this->movement.deceleration_rate);
         DATA.write(this->stats.creature_size);
         DATA.write(this->biped_object_info.head);
         DATA.write(this->biped_object_info.hair);
         DATA.write(this->stats.injured_health_threshold);
         DATA.write(this->biped_object_info.shield);
         DATA.write(this->stats.attribute_regen.health);
         DATA.write(this->stats.attribute_regen.magicka);
         DATA.write(this->stats.attribute_regen.stamina);
         DATA.write(this->stats.unarmed.damage);
         DATA.write(this->stats.unarmed.reach);
         DATA.write(this->biped_object_info.body);
         DATA.write(this->stats.aim_angle_tolerance);
         DATA.write(this->movement.angular_tolerance);
         DATA.write(this->alt_flags);
         if (record.version() >= 43) {
            DATA.write(this->mount_data.climb_on_offset.x);
            DATA.write(this->mount_data.climb_on_offset.y);
            DATA.write(this->mount_data.climb_on_offset.z);
            DATA.write(this->mount_data.dismount_offset.x);
            DATA.write(this->mount_data.dismount_offset.y);
            DATA.write(this->mount_data.dismount_offset.z);
            DATA.write(this->mount_data.camera_offset.x);
            DATA.write(this->mount_data.camera_offset.y);
            DATA.write(this->mount_data.camera_offset.z);
         }
         DATA.close();
      }
      {
         record.open_next_subrecord('MNAM').close();
         this->by_sex.male.skeleton_nif.save(record, intfc, 'ANAM', 'MODT');
      }
      {
         record.open_next_subrecord('FNAM').close();
         this->by_sex.female.skeleton_nif.save(record, intfc, 'ANAM', 'MODT');
      }
      {
         constexpr const auto types = std::array{ 'WALK', 'RUN1', 'SNEK', 'BLD0', 'SWIM' };
         for (uint32_t type : types) {
            auto& MTNM = record.open_next_subrecord('MTNM');
            MTNM.write(types);
            MTNM.close();
         }
      }
      if (this->by_sex.male.voicetype || this->by_sex.female.voicetype) {
         auto& subrecord = record.open_next_subrecord('VTCK');
         subrecord.write(this->by_sex.male.voicetype);
         subrecord.write(this->by_sex.female.voicetype);
         subrecord.close();
      }
      if (this->by_sex.male.decapitate_armor || this->by_sex.female.decapitate_armor) {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->by_sex.male.decapitate_armor);
         subrecord.write(this->by_sex.female.decapitate_armor);
         subrecord.close();
      }
      if (this->by_sex.male.head_data.default_hair_color || this->by_sex.female.head_data.default_hair_color) {
         auto& subrecord = record.open_next_subrecord('HCLF');
         subrecord.write(this->by_sex.male.head_data.default_hair_color);
         subrecord.write(this->by_sex.female.head_data.default_hair_color);
         subrecord.close();
      }
      if (this->tint_layer_count > 0) {
         auto& TINL = record.open_next_subrecord('TINL');
         TINL.write(this->tint_layer_count);
         TINL.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('PNAM');
         subrecord.write(this->facegen_clamp.main);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('UNAM');
         subrecord.write(this->facegen_clamp.face);
         subrecord.close();
      }
      this->attack_data.save(record, intfc);
      {  // Body Data
         record.open_next_subrecord('NAM1').close();
         {
            record.open_next_subrecord('MNAM').close();
            this->by_sex.male.lighting_model.save(record, intfc, 'MODL', 'MODT');
         }
         {
            record.open_next_subrecord('FNAM').close();
            this->by_sex.female.lighting_model.save(record, intfc, 'MODL', 'MODT');
         }
      }
      {
         auto* form = this->body_part_data.get_form_stub();
         // TODO: Fall back to a default?
         record.write_formID_subrecord('GNAM', form, true);
      }
      {  // Behavior Graphs
         record.open_next_subrecord('NAM3').close();
         {
            record.open_next_subrecord('MNAM').close();
            this->by_sex.male.behavior_graph.save(record, intfc, 'MODL', 'MODT');
         }
         {
            record.open_next_subrecord('FNAM').close();
            this->by_sex.female.behavior_graph.save(record, intfc, 'MODL', 'MODT');
         }
      }
      record.write_formID_subrecord('NAM4', this->material_type, true);
      record.write_formID_subrecord('NAM5', this->impact_data_set, true);
      record.write_formID_subrecord('NAM7', this->decapitation_effect, true);
      record.write_formID_subrecord('ONAM', this->container_sounds.open, true);
      record.write_formID_subrecord('LNAM', this->container_sounds.open, true);
      for (size_t i = 0; i < this->biped_object_info.names.size(); ++i) {
         auto& name = this->biped_object_info.names[i];
         if (name.size() > max_biped_object_name_length) {
            auto notice = specific_save_errors::biped_object_name_is_too_long(
               *intfc.target_stub,
               name.size(),
               i
            );
            intfc.throw_save_error(notice);
         }
         record.write_string_subrecord('NAME', name);
      }
      for (auto& item : this->movement.overrides) {
         record.write_formID_subrecord('MTYP', item.type, false);
         auto& SPED = record.open_next_subrecord('SPED');
         SPED.write(item.values.list);
         SPED.close();
      }
      {
         auto& VNAM = record.open_next_subrecord('VNAM');
         VNAM.write(this->equipment.flags);
         VNAM.close();
         for (auto& form : this->equipment.equip_slots)
            record.write_formID_subrecord('QNAM', form, false);
         record.write_formID_subrecord('UNES', this->equipment.unarmed_equip_slot, true);
      }
      {
         for (auto& name : this->phonemes.morph_names)
            record.write_string_subrecord('PHTN', name);
         for (auto& list : this->phonemes.weights) {
            auto& PHWT = record.open_next_subrecord('PHWT');
            for (float f : list)
               PHWT.write(f);
            PHWT.close();
         }
      }
      record.write_formID_subrecord('WKMV', this->movement.types.walk, true);
      record.write_formID_subrecord('RNMV', this->movement.types.run, true);
      record.write_formID_subrecord('SWMV', this->movement.types.swim, true);
      record.write_formID_subrecord('FLMV', this->movement.types.fly, true);
      record.write_formID_subrecord('SNMV', this->movement.types.sneak, true);
      record.write_formID_subrecord('SPMV', this->movement.types.sprint, true);
      {  // Head Data
         auto _save_head = [this, &record, &intfc](sex s) {
            auto& data = this->by_sex[s];
            auto& head = data.head_data;
            record.open_next_subrecord('NAM0').close();
            record.open_next_subrecord(s == sex::male ? 'MNAM' : 'FNAM').close();
            for (size_t i = 0; i < head.head_parts.size(); ++i) {
               auto& INDX = record.open_next_subrecord('INDX');
               INDX.write((uint32_t)i);
               INDX.close();
               record.write_formID_subrecord('HEAD', head.head_parts[i], false);
            }
            for (size_t i = 0; i < 4; ++i) {
               auto& MPAI = record.open_next_subrecord('MPAI');
               MPAI.write((uint32_t)i);
               MPAI.close();
               auto& MPAV = record.open_next_subrecord('MPAV');
               {
                  const cobb::bitset<256>* src = nullptr;
                  switch (i) {
                     case 0: src = &head.morphs.noses;  break;
                     case 1: src = &head.morphs.brows;  break;
                     case 2: src = &head.morphs.eyes;   break;
                     case 3: src = &head.morphs.mouths; break;
                  }
                  assert(src != nullptr);
                  for (size_t i = 0; i + 31 < 256; i += 32) {
                     uint32_t span = src->get_span<uint32_t>(i);
                     MPAV.write(span);
                  }
               }
               MPAV.close();
            }
            {
               auto& list = head.preset_actors;
               if (!list.empty()) {
                  auto signature = (s == sex::male) ? 'RPRM' : 'RPRF';
                  for (auto& item : list)
                     record.write_formID_subrecord(signature, item);
               }
            }
            {
               auto& list = head.hair_colors;
               if (!list.empty()) {
                  auto signature = (s == sex::male) ? 'ACHM' : 'ACHF';
                  for (auto& item : list)
                     record.write_formID_subrecord(signature, item);
               }
            }
            //
            // NOTE: The default hair color was serialized earlier.
            //
            record.write_formID_subrecord(s == sex::male ? 'DFTM' : 'DFTF', head.default_face_texture, true);
            for (auto& tint : head.face_tints) {
               auto& TINI = record.open_next_subrecord('TINI');
               TINI.write(tint.index);
               TINI.close();
               if (tint.type != face_tint_type::none) {
                  auto& TINP = record.open_next_subrecord('TINP');
                  TINP.write(tint.type);
                  TINP.close();
               }
               record.write_formID_subrecord('TIND', tint.default_color, true);
               for (auto& item : tint.presets) {
                  record.write_formID_subrecord('TINC', item.color, false);
                  auto& TINV = record.open_next_subrecord('TINV');
                  TINV.write(item.alpha);
                  TINV.close();
                  auto& TIRS = record.open_next_subrecord('TIRS');
                  TIRS.write(item.index);
                  TIRS.close();
               }
            }
         };
         auto _should_save_head = [](const sex_data& data) -> bool {
            auto& head = data.head_data;
            if (head.default_face_texture || head.default_hair_color)
               return true;
            if (!head.face_textures.empty())
               return true;
            if (!head.face_tints.empty())
               return true;
            if (!head.hair_colors.empty())
               return true;
            if (!head.head_parts.empty())
               return true;
            if (!head.preset_actors.empty())
               return true;
            if (auto& morph = head.morphs.brows; !morph.all())
               return true;
            if (auto& morph = head.morphs.eyes; !morph.all())
               return true;
            if (auto& morph = head.morphs.noses; !morph.all())
               return true;
            if (auto& morph = head.morphs.mouths; !morph.all())
               return true;
            return false;
         };

         if (_should_save_head(this->by_sex.male))
            _save_head(sex::male);
         if (_should_save_head(this->by_sex.female))
            _save_head(sex::female);
      }
      record.write_formID_subrecord('NAM8', this->morph_race, true);
      record.write_formID_subrecord('RNAM', this->armor_race, true);
   }
   void Race::_clear_impl() noexcept {
      this->attack_data.clear(*this);
      this->biped_object.clear(*this);
      this->keywords.clear(*this);
      this->script_data.clear(*this);
      this->spells.clear(*this);

      this->name.reset();
      this->description.reset();

      this->race_flags = {};
      this->alt_flags  = {};

      this->armor_race.set(*this, nullptr);
      this->body_part_data.set(*this, nullptr);
      this->decapitation_effect.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->material_type.set(*this, nullptr);
      this->morph_race.set(*this, nullptr);
      this->skin.set(*this, nullptr);

      this->tint_layer_count  = 0;
      this->biped_object_info = {};
      for (auto& sex : this->by_sex) {
         sex.behavior_graph.clear();
         sex.decapitate_armor.set(*this, nullptr);
         {
            auto& head = sex.head_data;
            head.default_face_texture.set(*this, nullptr);
            head.default_hair_color.set(*this, nullptr);
            clear_form_reference_list(head.face_textures, *this);
            for (auto& tint : head.face_tints) {
               tint.default_color.set(*this, nullptr);
               for (auto& preset : tint.presets)
                  preset.color.set(*this, nullptr);
            }
            head.face_tints.clear();
            clear_form_reference_list(head.hair_colors, *this);
            clear_form_reference_list(head.head_parts, *this);
            clear_form_reference_list(head.preset_actors, *this);
         }
         sex.lighting_model.clear();
         sex.skeleton_nif.clear();
         sex.voicetype.set(*this, nullptr);
      }
      {
         auto& dst = this->container_sounds;
         dst.open.set(*this, nullptr);
         dst.close.set(*this, nullptr);
      }
      {
         auto& dst = this->equipment;
         dst.flags = this->equipment.flags;
         clear_form_reference_list(dst.equip_slots, *this);
         dst.unarmed_equip_slot.set(*this, nullptr);
      }
      this->facegen_clamp = {};
      {
         auto& dst = this->movement;
         for (size_t i = 0; i < dst.types.list.size(); ++i) {
            dst.types.list[i].set(*this, nullptr);
         }
         size_t size = dst.overrides.size();
         for (auto& e : dst.overrides)
            e.type.set(*this, nullptr);
         dst.overrides.clear();
      }
      this->phonemes = {};
      this->stats = {};
      this->mount_data = {};
   }
   void Race::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->attack_data.sever_outbound_references_to(other, *this);
      this->biped_object.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->spells.sever_outbound_references_to(other, *this);

      this->armor_race.clear_if(*this, other);
      this->body_part_data.clear_if(*this, other);
      this->decapitation_effect.clear_if(*this, other);
      this->impact_data_set.clear_if(*this, other);
      this->material_type.clear_if(*this, other);
      this->morph_race.clear_if(*this, other);
      this->skin.clear_if(*this, other);

      for (auto& sex : this->by_sex) {
         sex.behavior_graph.sever_outbound_references_to(other, *this);
         sex.decapitate_armor.clear_if(*this, other);
         {
            auto& head = sex.head_data;
            head.default_face_texture.clear_if(*this, other);
            head.default_hair_color.clear_if(*this, other);
            remove_form_from_reference_list(head.face_textures, other, *this);
            for (auto& tint : head.face_tints) {
               tint.default_color.clear_if(*this, other);

               bool any_lost = false;
               for (auto& preset : tint.presets) {
                  if (preset.color == &other) {
                     preset.color.set(*this, nullptr);
                     any_lost = true;
                  }
               }
               if (any_lost) {
                  std::erase_if(tint.presets, [](const auto& e) -> bool { return e.color == nullptr; });
               }
            }
            remove_form_from_reference_list(head.hair_colors, other, *this);
            remove_form_from_reference_list(head.head_parts, other, *this);
            remove_form_from_reference_list(head.preset_actors, other, *this);
         }
         sex.lighting_model.sever_outbound_references_to(other, *this);
         sex.skeleton_nif.sever_outbound_references_to(other, *this);
         sex.voicetype.clear_if(*this, other);
      }

      this->container_sounds.open.clear_if(*this, other);
      this->container_sounds.close.clear_if(*this, other);
      {
         auto& dst = this->equipment;
         remove_form_from_reference_list(dst.equip_slots, other, *this);
         dst.unarmed_equip_slot.clear_if(*this, other);
      }
      {
         auto& dst = this->movement;
         for (size_t i = 0; i < dst.types.list.size(); ++i) {
            dst.types.list[i].clear_if(*this, other);
         }
         size_t size = dst.overrides.size();
         for(auto& e : dst.overrides)
            e.type.clear_if(*this, other);
      }
   }
}