#include "ActorBase.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ActorBase::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      uint8_t  esp1B = 0;
      uint16_t espF8 = 0;
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
               //
            #pragma region Components
            case 'ATKD':
            case 'ATKR':
               this->attack_data.load(record, intfc);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord, intfc);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               this->destruction_data.load(subrecord, intfc);
               break;
            case 'KSIZ':
            case 'KWDA':
               this->keywords.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            #pragma endregion
            #pragma region Creature sounds
            case 'CSDT':
               {
                  creature_sound entry;
                  if (subrecord.read(entry.type))
                     this->creature_sounds.push_back(entry);
               }
               break;
            case 'CSDI':
               if (!this->creature_sounds.empty()) {
                  auto& form = this->creature_sounds.back().sound;
                  if (subrecord.read(form)) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, form)
                     );
                  }
               }
               break;
            case 'CDSC':
               if (!this->creature_sounds.empty())
                  subrecord.read(this->creature_sounds.back().chance);
               break;
            #pragma endregion
            #pragma region Package override lists
            case 'SCOR':
               if (subrecord.read(this->package_override_lists.spectator)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->package_override_lists.spectator)
                  );
               }
               break;
            case 'OCOR':
               if (subrecord.read(this->package_override_lists.observe_corpse)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->package_override_lists.observe_corpse)
                  );
               }
               break;
            case 'GWOR':
               if (subrecord.read(this->package_override_lists.guard_warn)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->package_override_lists.guard_warn)
                  );
               }
               break;
            case 'ECOR':
               if (subrecord.read(this->package_override_lists.combat)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->package_override_lists.combat)
                  );
               }
               break;
            #pragma endregion
            #pragma region Perks
            case 'PRKZ':
               {
                  uint32_t size;
                  if (subrecord.read(size))
                     this->perks.reserve(size);
               }
               break;
            case 'PRKR':
               if (subrecord.read(form_id)) {
                  this->perks.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::perk, this->stub, form_id)
                  );
               }
               break;
            #pragma endregion
            #pragma region Spells (TESSpellList)
            case 'SPCT':
               {
                  uint32_t size;
                  if (subrecord.read(size))
                     this->spells.reserve(size);
               }
               break;
            case 'SPLO':
               if (subrecord.read(form_id)) {
                  this->spells.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::spell, this->stub, form_id)
                  );
               }
               break;
            #pragma endregion
            #pragma region Tint layers
            case 'TINI':
               subrecord.read(this->tint_layers.emplace_back().index);
               break;
            case 'TIAS':
               if (!this->tint_layers.empty())
                  subrecord.read(this->tint_layers.back().preset);
               break;
            case 'TINC':
               if (!this->tint_layers.empty())
                  this->tint_layers.back().color.load(subrecord);
               break;
            case 'TINV':
               if (!this->tint_layers.empty())
                  subrecord.read(this->tint_layers.back().interpolation);
               break;
            #pragma endregion
               //
            case 'DATA':
               //
               // The game takes this subrecord as an opportunity to load any of TESValueForm, TESWeightForm, 
               // TESHealthForm, and TESAttackDamageForm, but the ActorBase class doesn't inherit from any of 
               // them, so the subrecord is effectively skipped.
               //
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'SHRT':
               subrecord.to_string(this->short_name);
               break;
            case 'ACBS':
               if (record.version() < 0x1D) {
                  if (subrecord.is_in_bounds(0x1C)) {
                     subrecord.unchecked_read(this->base_stats.flags);          // 00 -> 00
                     subrecord.unchecked_read(this->base_stats.magicka_offset); // 04 -> 04
                     subrecord.unchecked_read(this->base_stats.stamina_offset); // 06 -> 06
                     subrecord.skip_bytes(2);                                   // 08
                     subrecord.unchecked_read(this->base_stats.level);          // 0A -> 08
                     subrecord.unchecked_read(this->base_stats.calc_min_level); // 0C -> 0A
                     subrecord.unchecked_read(this->base_stats.calc_max_level); // 0E -> 0C
                     subrecord.unchecked_read(this->base_stats.speed_mult);     // 10 -> 0E
                     subrecord.unchecked_read(this->base_stats.disposition);    // 12 -> 10
                     subrecord.unchecked_read(this->base_stats.template_flags); // 14 -> 12
                     subrecord.unchecked_read(this->base_stats.health_offset);  // 16 -> 14
                     subrecord.unchecked_read(this->base_stats.bleedout_override); // 18 -> 16
                  }
                  break;
               }
               if (subrecord.is_in_bounds(0x18)) {
                  subrecord.unchecked_read(this->base_stats.flags);
                  subrecord.unchecked_read(this->base_stats.magicka_offset);
                  subrecord.unchecked_read(this->base_stats.stamina_offset);
                  subrecord.unchecked_read(this->base_stats.level);
                  subrecord.unchecked_read(this->base_stats.calc_min_level);
                  subrecord.unchecked_read(this->base_stats.calc_max_level);
                  subrecord.unchecked_read(this->base_stats.speed_mult);
                  subrecord.unchecked_read(this->base_stats.disposition);
                  subrecord.unchecked_read(this->base_stats.template_flags);
                  subrecord.unchecked_read(this->base_stats.health_offset);
                  subrecord.unchecked_read(this->base_stats.bleedout_override);
               }
               break;
            case 'AIDT':
               {
                  decltype(ai_data) aid;
                  if (subrecord.is_in_bounds(0x14)) {
                     subrecord.read(aid.aggression);
                     subrecord.read(aid.confidence);
                     subrecord.read(aid.energy_level);
                     subrecord.read(aid.morality);
                     subrecord.read(aid.mood);
                     subrecord.read(aid.assistance);
                     subrecord.read(aid.aggression);
                     subrecord.read(aid.aggro.use_radius);
                     subrecord.skip_bytes(1);
                     subrecord.read(aid.aggro.warn);
                     if (record.version() < 0x1D) {
                        uint32_t unused;
                        subrecord.read(unused);
                        subrecord.read(aid.aggro.attack);
                     } else {
                        subrecord.read(aid.aggro.warn_attack);
                        subrecord.read(aid.aggro.attack);
                     }
                  }
                  //
                  if (record.version() < 4) {
                     if (aid.aggression > 1)
                        ++aid.aggression;
                  }
                  if (record.version() < 6) {
                     switch ((uint8_t)aid.aggression) { // wait... is this a one-based version of the confidence enum?
                        case 0:
                           break;
                        case 1:
                           aid.aggression = aggression::unaggressive;
                           aid.assistance = assistance::helps_allies;
                           break;
                        case 2:
                           aid.aggression = aggression::aggressive;
                           aid.assistance = assistance::helps_allies;
                           break;
                        case 3:
                           aid.aggression = aggression::aggressive;
                           aid.assistance = assistance::helps_friends_and_allies;
                           break;
                        case 4:
                           aid.aggression = aggression::very_aggressive;
                           aid.assistance = assistance::helps_friends_and_allies;
                           break;
                        case 5:
                           aid.aggression = aggression::frenzied;
                           aid.assistance = assistance::helps_nobody;
                           break;
                     }
                  }
                  if (record.version() < 7) {
                     aid.confidence = 4 - aid.confidence;
                  }
                  if (record.version() < 0x21) {
                     auto val = aid.aggro.warn;
                     aid.aggro.warn        = 0;
                     aid.aggro.warn_attack = val;
                     aid.aggro.attack      = val >> 2;
                  }
               }
               break;
            case 'ANAM':
               if (subrecord.read(this->far_away_model)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::armor, this->stub, this->far_away_model)
                  );
               }
               break;
            case 'CNAM':
               if (subrecord.read(this->combat_class)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::combat_class, this->stub, this->combat_class)
                  );
               }
               break;
            case 'DNAM':
               if (subrecord.is_in_bounds(0x34)) {
                  for (auto& byte : this->base_data.skill_values.list)
                     subrecord.unchecked_read(byte);
                  for (auto& byte : this->base_data.skill_offsets.list)
                     subrecord.unchecked_read(byte);
                  subrecord.unchecked_read(this->base_data.health);
                  subrecord.unchecked_read(this->base_data.magicka);
                  subrecord.unchecked_read(this->base_data.stamina);
                  subrecord.skip_bytes(2);
                  subrecord.unchecked_read(this->base_data.far_away_model_distance);
                  subrecord.unchecked_read(this->base_data.geared_up_weapons);
               }
               break;
            case 'FNAM':
               subrecord.read(espF8);
               break;
            case 'GNAM':
               if (subrecord.read(this->gift_filter)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::formlist, this->stub, this->gift_filter)
                  );
               }
               break;
            case 'INAM':
               if (subrecord.read(this->death_item)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::leveled_item, this->stub, this->death_item)
                  );
               }
               break;
            case 'QNAM':
               if (subrecord.is_in_bounds(0xC)) {
                  subrecord.unchecked_read(this->texture_lighting.r);
                  subrecord.unchecked_read(this->texture_lighting.g);
                  subrecord.unchecked_read(this->texture_lighting.b);
               }
               break;
            case 'RNAM':
               if (subrecord.read(this->race)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::race, this->stub, this->race)
                  );
               }
               break;
            case 'SNAM':
               {
                  faction_membership entry;
                  if (subrecord.read(entry)) {
                     this->faction_memberships.push_back(entry);
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::faction, this->stub, entry.faction)
                     );
                  }
               }
               break;
            case 'WNAM':
               if (subrecord.read(this->worn_armor)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::armor, this->stub, this->worn_armor)
                  );
               }
               break;
            case 'ZNAM':
               if (subrecord.read(this->combat_style)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::combat_style, this->stub, this->combat_style)
                  );
               }
               break;
            case 'CRIF':
               if (subrecord.read(this->crime_faction)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::faction, this->stub, this->crime_faction)
                  );
               }
               break;
            case 'DOFT':
               if (subrecord.read(this->outfits.default)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::outfit, this->stub, this->outfits.default)
                  );
               }
               break;
            case 'FTST':
               if (subrecord.read(this->face_texture_set)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::texture_set, this->stub, this->face_texture_set)
                  );
               }
               break;
            case 'SOFT':
               if (subrecord.read(this->outfits.sleeping)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::outfit, this->stub, this->outfits.sleeping)
                  );
               }
               break;
            case 'DPLT':
               if (subrecord.read(this->default_package_list)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, this->stub, this->default_package_list)
                  );
               }
               break;
            case 'TPLT':
               if (subrecord.read(this->template_actor)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), { form_type::actor_base, form_type::leveled_character }, this->stub, this->template_actor)
                  );
               }
               break;
            case 'HEAD': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'ENAM': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'PNAM':
               if (subrecord.read(form_id))
                  this->head_parts.push_back(form_id);
               break;
            case 'PKID':
               if (subrecord.read(form_id)) {
                  this->package_list.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, this->stub, form_id)
                  );
               }
               break;
            case 'HCLF':
               if (subrecord.read(form_id)) {
                  this->hair_colors.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::color, this->stub, form_id)
                  );
               }
               break;
            case 'VTCK':
               if (subrecord.read(this->voicetype)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::voicetype, this->stub, this->voicetype)
                  );
               }
               break;
            case 'NAM0':
            case 'NAM1':
            case 'NAM2':
            case 'NAM3':
               if (esp1B == 0)
                  esp1B = 1;
               break;
            case 'NAM6':
               if (subrecord.read(this->height))
                  if (this->height == 0.0F) // game does this, too
                     this->height = 1.0F;
               break;
            case 'NAM7':
               subrecord.read(this->weight);
               break;
            case 'NAM8': // sound level
               if (subrecord.read(this->sound_level))
                  if (this->sound_level > 4) // game does this, too
                     this->sound_level = 0;
               break;
            case 'NAM9':
               if (subrecord.is_in_bounds(0x4C)) {
                  //
                  // If all of these floats are 0, then the TESNPC instance doesn't even bother 
                  // allocating storage for them.
                  //
                  subrecord.unchecked_read(this->face_morphs.nose.length);
                  subrecord.unchecked_read(this->face_morphs.nose.height);
                  subrecord.unchecked_read(this->face_morphs.jaw.height);
                  subrecord.unchecked_read(this->face_morphs.jaw.width);
                  subrecord.unchecked_read(this->face_morphs.jaw.depth);
                  subrecord.unchecked_read(this->face_morphs.cheeks.height);
                  subrecord.unchecked_read(this->face_morphs.cheeks.depth);
                  subrecord.unchecked_read(this->face_morphs.eyes.height);
                  subrecord.unchecked_read(this->face_morphs.eyes.width);
                  subrecord.unchecked_read(this->face_morphs.brows.height);
                  subrecord.unchecked_read(this->face_morphs.brows.width);
                  subrecord.unchecked_read(this->face_morphs.brows.depth);
                  subrecord.unchecked_read(this->face_morphs.lips.height);
                  subrecord.unchecked_read(this->face_morphs.lips.depth);
                  subrecord.unchecked_read(this->face_morphs.chin.width);
                  subrecord.unchecked_read(this->face_morphs.chin.height);
                  subrecord.unchecked_read(this->face_morphs.chin.depth);
                  subrecord.unchecked_read(this->face_morphs.eyes.depth);
                  subrecord.unchecked_read(this->face_morphs.unknown);
               }
               break;
         }
         static_assert(false, "FINISH ME");
      }
   }
   /*static*/ void ActorBase::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      form_id_t race;
      form_id_t template_actor;
      form_id_t voicetype;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'RNAM': // race
            case 'TPLT': // template actor base
            case 'VTCK': // voicetype
            case 'INAM': // death item
            case 'SNAM': // faction (has four more bytes, but we don't need them)
            case 'SPLO': // spell
            case 'WNAM': // skin
            case 'ANAM': // far-away skin
            case 'ATKR': // attack race
            case 'SPOR': // spectator package override
            case 'OCOR': // observe corpse package override
            case 'GWOR': // guard worn package override
            case 'ECOR': // combat package override
            case 'PRKR': // perk
            case 'PKID': // package
            case 'CNAM': // class
            case 'PNAM': // head part
            case 'HCLF': // hair color form
            case 'ZNAM': // combat style
            case 'GNAM': // gift filter list
            case 'CSDI': // sound
            case 'CSCR': // audio template
            case 'DOFT': // default outfit
            case 'SOFT': // sleep outfit
            case 'DPLT': // default package list
            case 'CRIF': // crime faction
            case 'FTST': // face textureset
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'ATKD': // attack data
               subrecord.skip_bytes(8);
               if (subrecord.read(formID)) // attack spell
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(16);
               if (subrecord.read(formID)) // attack type
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, uib);
               break;
            //
            // End of destruction stage fields.
            //
            case 'ACBS': // character base stats
            case 'ATKE': // attack event
            case 'SPCT': // spell count
            case 'PRKZ': // perk count
            case 'COCT': // item count ("count of container")
            case 'AIDT': // AI data
            case 'FULL': // full name
            case 'SHRT': // short name
            case 'DATA': // marker for DNAM position
            case 'DNAM': // skill/stat data
            case 'NAM5': // unknown two-byte int
            case 'NAM6': // height
            case 'NAM7': // weight
            case 'NAM8': // sound level
            case 'CSDT': // sound type
            case 'CSDC': // sound chance
            case 'QNAM': // skin tone
            case 'NAM9': // face morphs values
            case 'NAMA': // face part integers
            case 'TINI': // tint item
            case 'TINC': // tint color
            case 'TINV': // tint value
            case 'TIAS': // unknown two-byte int
               break;
         }
      }
   }
}