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
            case 'ATKD': // not checking for ATKE is intentional. the NPC_ loader doesn't check for it, so if it appears anywhere other than after ATKD (such that the ATKD loader handles it), then it is unrecognized
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
               if (subrecord.read(this->ai.package_override_lists.spectator)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->ai.package_override_lists.spectator)
                  );
               }
               break;
            case 'OCOR':
               if (subrecord.read(this->ai.package_override_lists.observe_corpse)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->ai.package_override_lists.observe_corpse)
                  );
               }
               break;
            case 'GWOR':
               if (subrecord.read(this->ai.package_override_lists.guard_warn)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->ai.package_override_lists.guard_warn)
                  );
               }
               break;
            case 'ECOR':
               if (subrecord.read(this->ai.package_override_lists.combat)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, this->ai.package_override_lists.combat)
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
               subrecord.read(this->name);
               break;
            case 'SHRT':
               subrecord.read(this->short_name);
               break;
            case 'ACBS':
               if (record.version() < 0x1D) {
                  if (subrecord.is_in_bounds(0x1C)) {
                     subrecord.unchecked_read(this->actor_flags);
                     subrecord.unchecked_read(this->stats.offsets.magicka);
                     subrecord.unchecked_read(this->stats.offsets.stamina);
                     subrecord.skip_bytes(2);                                   // 08
                     subrecord.unchecked_read(this->stats.level);          // 0A -> 08
                     subrecord.unchecked_read(this->stats.calc_min_level); // 0C -> 0A
                     subrecord.unchecked_read(this->stats.calc_max_level); // 0E -> 0C
                     subrecord.unchecked_read(this->stats.speed_mult);     // 10 -> 0E
                     subrecord.unchecked_read(this->stats.disposition);    // 12 -> 10
                     subrecord.unchecked_read(this->template_data.flags); // 14 -> 12
                     subrecord.unchecked_read(this->stats.offsets.health);  // 16 -> 14
                     subrecord.unchecked_read(this->stats.bleedout_threshold); // 18 -> 16
                  }
                  break;
               }
               if (subrecord.is_in_bounds(0x18)) {
                  subrecord.unchecked_read(this->actor_flags);
                  subrecord.unchecked_read(this->stats.offsets.magicka);
                  subrecord.unchecked_read(this->stats.offsets.stamina);
                  subrecord.unchecked_read(this->stats.level);
                  subrecord.unchecked_read(this->stats.calc_min_level);
                  subrecord.unchecked_read(this->stats.calc_max_level);
                  subrecord.unchecked_read(this->stats.speed_mult);
                  subrecord.unchecked_read(this->stats.disposition);
                  subrecord.unchecked_read(this->template_data.flags);
                  subrecord.unchecked_read(this->stats.offsets.health);
                  subrecord.unchecked_read(this->stats.bleedout_threshold);
               }
               break;
            case 'TPLT':
               if (subrecord.read(this->template_data.actor)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), { form_type::actor_base, form_type::leveled_character }, this->stub, this->template_data.actor)
                  );
               }
               break;
            case 'AIDT':
               {
                  uint8_t aggression;
                  uint8_t assistance;
                  uint8_t confidence;
                  if (subrecord.is_in_bounds(0x14)) {
                     subrecord.read(aggression);                // 00
                     subrecord.read(confidence);                // 01
                     subrecord.read(this->ai.energy_level);     // 02
                     subrecord.read(this->ai.morality);         // 03
                     subrecord.read(this->ai.mood);             // 04
                     subrecord.read(assistance);                // 05
                     subrecord.read(this->ai.aggro.use_radius); // 06
                     subrecord.skip_bytes(1);                   // 07
                     subrecord.read(this->ai.aggro.warn);       // 08
                     if (record.version() < 0x1D) {
                        uint32_t unused;
                        subrecord.read(unused);                 // 0C
                        subrecord.read(this->ai.aggro.attack);  // 10
                     } else {
                        subrecord.read(this->ai.aggro.warn_attack); // 0C
                        subrecord.read(this->ai.aggro.attack);      // 10
                     }
                  }
                  //
                  if (record.version() < 4) {
                     if (aggression > 1)
                        ++aggression;
                  }
                  if (record.version() < 6) {
                     switch (aggression) { // wait... is this a one-based version of the confidence enum?
                        case 0:
                           break;
                        case 1:
                           aggression = (uint8_t)aggression::unaggressive;
                           assistance = (uint8_t)assistance::helps_allies;
                           break;
                        case 2:
                           aggression = (uint8_t)aggression::aggressive;
                           assistance = (uint8_t)assistance::helps_allies;
                           break;
                        case 3:
                           aggression = (uint8_t)aggression::aggressive;
                           assistance = (uint8_t)assistance::helps_friends_and_allies;
                           break;
                        case 4:
                           aggression = (uint8_t)aggression::very_aggressive;
                           assistance = (uint8_t)assistance::helps_friends_and_allies;
                           break;
                        case 5:
                           aggression = (uint8_t)aggression::frenzied;
                           assistance = (uint8_t)assistance::helps_nobody;
                           break;
                     }
                  }
                  if (record.version() < 7) {
                     confidence = 4 - confidence;
                  }
                  if (record.version() < 0x21) {
                     auto val = this->ai.aggro.warn;
                     this->ai.aggro.warn        = 0;
                     this->ai.aggro.warn_attack = val;
                     this->ai.aggro.attack      = val >> 2;
                  }
                  this->ai.aggression = (ActorBase::aggression)aggression;
                  this->ai.assistance = (ActorBase::assistance)assistance;
                  this->ai.confidence = (ActorBase::confidence)confidence;
               }
               break;
            case 'ANAM':
               if (subrecord.read(this->far_away.model)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::armor, this->stub, this->far_away.model)
                  );
               }
               break;
            case 'CNAM':
               if (subrecord.read(this->stats.combat_class)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::combat_class, this->stub, this->stats.combat_class)
                  );
               }
               break;
            case 'DNAM':
               if (subrecord.is_in_bounds(0x34)) {
                  for (auto& byte : this->stats.base.skills.list)
                     subrecord.unchecked_read(byte);
                  for (auto& byte : this->stats.offsets.skills.list)
                     subrecord.unchecked_read(byte);
                  subrecord.unchecked_read(this->stats.base.health);
                  subrecord.unchecked_read(this->stats.base.magicka);
                  subrecord.unchecked_read(this->stats.base.stamina);
                  subrecord.skip_bytes(2);
                  subrecord.unchecked_read(this->far_away.distance);
                  subrecord.unchecked_read(this->geared_up_weapons);
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
               if (subrecord.is_in_bounds(5)) {
                  auto& entry = this->faction_memberships.emplace_back();
                  subrecord.unchecked_read(entry.faction);
                  subrecord.unchecked_read(entry.rank);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::faction, this->stub, entry.faction)
                  );
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
               if (subrecord.read(this->stats.combat_style)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::combat_style, this->stub, this->stats.combat_style)
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
               if (subrecord.read(this->outfits.normal)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::outfit, this->stub, this->outfits.normal)
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
               if (subrecord.read(this->ai.default_package_list)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, this->stub, this->ai.default_package_list)
                  );
               }
               break;
            case 'FTST':
               if (subrecord.read(this->face.texture_set)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::texture_set, this->stub, this->face.texture_set)
                  );
               }
               break;
            case 'HEAD': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'ENAM': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'PNAM':
               if (subrecord.read(form_id))
                  this->head.head_parts.push_back(form_id);
               break;
            case 'PKID':
               if (subrecord.read(form_id)) {
                  this->ai.package_list.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, this->stub, form_id)
                  );
               }
               break;
            case 'HCLF':
               if (subrecord.read(form_id)) {
                  this->head.hair_colors.push_back(form_id);
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
                  subrecord.unchecked_read(this->face.morphs.nose.length);
                  subrecord.unchecked_read(this->face.morphs.nose.height);
                  subrecord.unchecked_read(this->face.morphs.jaw.height);
                  subrecord.unchecked_read(this->face.morphs.jaw.width);
                  subrecord.unchecked_read(this->face.morphs.jaw.depth);
                  subrecord.unchecked_read(this->face.morphs.cheeks.height);
                  subrecord.unchecked_read(this->face.morphs.cheeks.depth);
                  subrecord.unchecked_read(this->face.morphs.eyes.height);
                  subrecord.unchecked_read(this->face.morphs.eyes.width);
                  subrecord.unchecked_read(this->face.morphs.brows.height);
                  subrecord.unchecked_read(this->face.morphs.brows.width);
                  subrecord.unchecked_read(this->face.morphs.brows.depth);
                  subrecord.unchecked_read(this->face.morphs.lips.height);
                  subrecord.unchecked_read(this->face.morphs.lips.depth);
                  subrecord.unchecked_read(this->face.morphs.chin.width);
                  subrecord.unchecked_read(this->face.morphs.chin.height);
                  subrecord.unchecked_read(this->face.morphs.chin.depth);
                  subrecord.unchecked_read(this->face.morphs.eyes.depth);
                  subrecord.unchecked_read(this->face.morphs.unknown);
               }
               break;
            case 'NAMA':
               if (subrecord.is_in_bounds(0x10)) {
                  subrecord.unchecked_read(this->face.parts.nose);
                  subrecord.unchecked_read(this->face.parts.unknown);
                  subrecord.unchecked_read(this->face.parts.eyes);
                  subrecord.unchecked_read(this->face.parts.mouth);
               }
               break;
         }
      }
   }
   /*static*/ void ActorBase::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      components::attack_data::use_info_state attack_data;
      form_id_t form_id;
      form_id_t combat_class;
      form_id_t combat_style;
      form_id_t crime_faction;
      form_id_t death_item;
      form_id_t default_package_list;
      form_id_t face_texture_set;
      form_id_t far_away_model;
      form_id_t gift_filter;
      struct {
         form_id_t normal; // default
         form_id_t sleeping;
      } outfits;
      form_id_t race;
      form_id_t template_actor;
      form_id_t voicetype;
      form_id_t worn_armor;
      struct {
         form_id_t spectator;      // SPOR
         form_id_t observe_corpse; // OCOR
         form_id_t guard_warn;     // GWOR
         form_id_t combat;         // ECOR
      } package_override_lists;
      bool      seen_any_creature_sound = false;
      form_id_t creature_sound;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
               //
            #pragma region Components
            case 'ATKD': // not checking for ATKE is intentional. the NPC_ loader doesn't check for it, so if it appears anywhere other than after ATKD (such that the ATKD loader handles it), then it is unrecognized
            case 'ATKR':
               attack_data.read(record);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, uib);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            #pragma endregion
            #pragma region Creature sounds
            case 'CSDT': // creature sound type
               uib.add_outbound_reference(creature_sound);
               creature_sound = 0;
               //
               seen_any_creature_sound = true;
               break;
            case 'CSDI': // last creature sound form
               if (seen_any_creature_sound)
                  subrecord.read(creature_sound);
               break;
            case 'CDSC': // last creature sound chance
               break;
            #pragma endregion
            #pragma region Package override lists
            case 'SCOR':
               subrecord.read(package_override_lists.spectator);
               break;
            case 'OCOR':
               subrecord.read(package_override_lists.observe_corpse);
               break;
            case 'GWOR':
               subrecord.read(package_override_lists.guard_warn);
               break;
            case 'ECOR':
               subrecord.read(package_override_lists.combat);
               break;
            #pragma endregion
            #pragma region Tint layers
            case 'TINI': // tint index
            case 'TIAS': // tint preset
            case 'TINC': // tint color
            case 'TINV': // tint value
               break;
            #pragma endregion
               //
            case 'PRKZ': // perk count (analogous to std::vector::reserve)
            case 'SPCT': // spell count (analogous to std::vector::reserve)
               break;
            case 'PRKR': // perk
            case 'SPLO': // spell
            case 'SNAM': // faction relationship (form ID + a byte that we can ignore here)
            case 'HEAD': // found via disassembly; identical to PNAM
               [[fallthrough]];
            case 'ENAM': // found via disassembly; identical to PNAM
               [[fallthrough]];
            case 'PNAM': // head-part array
            case 'PKID': // package array
            case 'HCLF': // hair color array
               if (subrecord.read(form_id))
                  uib.add_outbound_reference(form_id);
               break;
               //
            case 'DATA': // no-op
            case 'FULL': // name
            case 'SHRT': // short name
            case 'ACBS': // base stats
            case 'DNAM':
            case 'QNAM':
               break;
            case 'TPLT':
               subrecord.read(template_actor);
               break;
            case 'AIDT':
               break;
            case 'ANAM':
               subrecord.read(far_away_model);
               break;
            case 'CNAM':
               subrecord.read(combat_class);
               break;
            case 'GNAM':
               subrecord.read(gift_filter);
               break;
            case 'INAM':
               subrecord.read(death_item);
               break;
            case 'RNAM':
               subrecord.read(race);
               break;
            case 'WNAM':
               subrecord.read(worn_armor);
               break;
            case 'ZNAM':
               subrecord.read(combat_style);
               break;
            case 'CRIF':
               subrecord.read(crime_faction);
               break;
            case 'DOFT':
               subrecord.read(outfits.normal);
               break;
            case 'SOFT':
               subrecord.read(outfits.sleeping);
               break;
            case 'DPLT':
               subrecord.read(default_package_list);
               break;
            case 'FTST':
               subrecord.read(face_texture_set);
               break;
            case 'VTCK':
               subrecord.read(voicetype);
               break;
            case 'NAM0':
            case 'NAM1':
            case 'NAM2':
            case 'NAM3':
            case 'NAM6': // height
            case 'NAM7': // weight
            case 'NAM8': // sound level
            case 'NAM9': // face morphs
            case 'NAMA': // face parts
               break;
         }
      }
      attack_data.commit(uib);
   }
}