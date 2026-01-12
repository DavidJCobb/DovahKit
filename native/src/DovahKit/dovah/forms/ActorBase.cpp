#include "ActorBase.h"
#include "_common_cpp.h"

#include "../utils/get_all_relevant_template_actors.h"

namespace dovah::loaded_forms {
   void ActorBase::copy_data_from_template_actor() {
      auto relevant = get_all_relevant_template_actors(this->stub);
      if (relevant.empty())
         return;

      auto& list = relevant.templates.all;
      for (size_t i = 0; i < list.size(); ++i) {
         if (!list[i])
            continue;
         this->copy_data_from_actor(list[i]->stub, (template_flag::type)(1 << i));
      }
   }
   void ActorBase::copy_data_from_actor(form_stub& source, template_flag::type flag) {
      auto template_loaded = source.load().ptr_cast<ActorBase>();
      if (!template_loaded)
         return;

      auto& src_actor = *template_loaded;

      auto _copy_flag = [this, &src_actor](actor_flag::type flag) {
         if (src_actor.actor_flags & flag)
            this->actor_flags |= flag;
         else
            this->actor_flags &= ~flag;
      };
      auto _copy_form = [this, &src_actor](form_reference_t& dst, const form_reference_t& src) {
         dst.set(*this, src);
      };

      switch (flag) {
         case template_flag::use_traits:
            {  // Traits
               _copy_flag(actor_flag::female);
               _copy_flag(actor_flag::opposite_gender_animations);
               _copy_form(this->race,           src_actor.race);
               _copy_form(this->skin,           src_actor.skin);
               _copy_form(this->voicetype,      src_actor.voicetype);
               _copy_form(this->death_item,     src_actor.death_item);
               _copy_form(this->far_away.model, src_actor.far_away.model);
               this->far_away.distance = src_actor.far_away.distance;
               this->height = src_actor.height;
               this->weight = src_actor.weight;
               this->stats.disposition = src_actor.stats.disposition;
            }
            {  // Sounds
               this->sound_level = src_actor.sound_level;

               auto& dst = this->creature_sounds;
               auto& src = src_actor.creature_sounds;
               if (auto* stub = src.inherits_from()) {
                  dst.set_inherits_from(*this, stub);
               } else {
                  if (src.sound_count()) {
                     dst.replace_sounds(*this, src.sounds());
                  } else {
                     dst.replace_sounds(*this, {});
                  }
               }
            }
            {  // FaceGen
               _copy_form(this->facegen.complexion, src_actor.facegen.complexion);
               _copy_form(this->facegen.hair_color, src_actor.facegen.hair_color);
               copy_form_reference_list(*this, this->facegen.head_parts, src_actor.facegen.head_parts);
               this->tint_layers = src_actor.tint_layers;

               this->facegen.morphs = src_actor.facegen.morphs;
            }
            break;
         case template_flag::use_stats:
            {
               const auto& src = src_actor.stats;
               auto& dst = this->stats;

               _copy_flag(actor_flag::auto_calc_stats);
               _copy_flag(actor_flag::pc_level_mult);
               dst.level          = src.level;
               dst.calc_min_level = src.calc_min_level;
               dst.calc_max_level = src.calc_max_level;
               dst.attributes     = src.attributes;
               dst.skills         = src.skills;
               _copy_form(dst.combat_class, src.combat_class);
               dst.speed_mult     = src.speed_mult;
               _copy_flag(actor_flag::bleedout_override);
               dst.bleedout_threshold = src.bleedout_threshold;
            }
            break;
         case template_flag::use_factions:
            {
               const auto& src = src_actor.faction_memberships;
               auto& dst = this->faction_memberships;

               for (auto& item : dst)
                  item.faction.set(*this, nullptr);
               size_t size = src.size();
               dst.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  _copy_form(dst[i].faction, src[i].faction);
                  dst[i].rank = src[i].rank;
               }

               _copy_form(this->crime_faction, src_actor.crime_faction);
            }
            break;
         case template_flag::use_spells:
            this->spells.clone_from(src_actor.spells, *this);
            {  // Perks and ranks
               const auto& src = src_actor.perks;
               auto& dst = this->perks;

               for (auto& item : dst)
                  item.perk.set(*this, nullptr);
               size_t size = src.size();
               dst.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  _copy_form(dst[i].perk, src[i].perk);
                  dst[i].rank = src[i].rank;
               }
            }
            break;
         case template_flag::use_ai_data:
            {
               const auto& src = src_actor.ai;
               auto& dst = this->ai;

               dst.aggression   = src.aggression;
               dst.aggro        = src.aggro;
               dst.assistance   = src.assistance;
               dst.confidence   = src.confidence;
               dst.energy_level = src.energy_level;
               dst.mood         = src.mood;
               dst.morality     = src.morality;
            }
            break;
         case template_flag::use_ai_packages:
            copy_form_reference_list(*this, this->ai.package_list, src_actor.ai.package_list);
            break;
         case template_flag::use_package_overrides:
            {
               _copy_form(this->ai.default_package_list, src_actor.ai.default_package_list);

               const auto& src = src_actor.ai.package_override_lists;
               auto& dst = this->ai.package_override_lists;

               _copy_form(dst.combat,         src.combat);
               _copy_form(dst.guard_warn,     src.guard_warn);
               _copy_form(dst.observe_corpse, src.observe_corpse);
               _copy_form(dst.spectator,      src.spectator);
            }
            break;
         case template_flag::use_animations:
            //
            // ???
            //
            break;
         case template_flag::use_base_data:
            this->name       = src_actor.name;
            this->short_name = src_actor.short_name;
            _copy_flag(actor_flag::essential);
            _copy_flag(actor_flag::is_protected);
            _copy_flag(actor_flag::respawn);
            _copy_flag(actor_flag::summonable);
            _copy_flag(actor_flag::simple_actor);
            _copy_flag(actor_flag::doesnt_affect_stealth_meter);
            break;
         case template_flag::use_inventory:
            _copy_form(this->outfits.normal,   src_actor.outfits.normal);
            _copy_form(this->outfits.sleeping, src_actor.outfits.sleeping);
            this->inventory.clone_from(src_actor.inventory, *this);
            break;
         case template_flag::use_scripts:
            // Doesn't copy data.
            break;
         case template_flag::use_attack_data:
            this->attack_data.clone_from(src_actor.attack_data, *this);
            break;
         case template_flag::use_keywords:
            this->keywords.clone_from(src_actor.keywords, *this);
            break;
      }
   }

   void ActorBase::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
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
            case components::attack_data::subrecord_signature_race:
            case components::attack_data::subrecord_signature_data:
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
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case components::spell_list::subrecord_signature_count:
            case components::spell_list::subrecord_signature_entry:
               this->spells.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            #pragma endregion
               //
            case structs::actor_creature_sounds::subrecord_signature_inherit:
            case structs::actor_creature_sounds::subrecord_signature_sound_chance:
            case structs::actor_creature_sounds::subrecord_signature_sound_form:
            case structs::actor_creature_sounds::subrecord_signature_sound_start:
               this->creature_sounds.load(subrecord, intfc);
               break;
            #pragma region Package override lists
            case 'SCOR':
               if (subrecord.read(this->ai.package_override_lists.spectator)) {
                  intfc.warn_if_ref_is_wrong_type(this->ai.package_override_lists.spectator, form_type::formlist, subrecord.signature());
               }
               break;
            case 'OCOR':
               if (subrecord.read(this->ai.package_override_lists.observe_corpse)) {
                  intfc.warn_if_ref_is_wrong_type(this->ai.package_override_lists.observe_corpse, form_type::formlist, subrecord.signature());
               }
               break;
            case 'GWOR':
               if (subrecord.read(this->ai.package_override_lists.guard_warn)) {
                  intfc.warn_if_ref_is_wrong_type(this->ai.package_override_lists.guard_warn, form_type::formlist, subrecord.signature());
               }
               break;
            case 'ECOR':
               if (subrecord.read(this->ai.package_override_lists.combat)) {
                  intfc.warn_if_ref_is_wrong_type(this->ai.package_override_lists.combat, form_type::formlist, subrecord.signature());
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
                  intfc.warn_if_ref_is_wrong_type(form_id, form_type::perk, subrecord.signature());

                  auto& entry = this->perks.emplace_back();
                  entry.perk.unmanaged_set(form_id.get_form_stub());
                  subrecord.read(entry.rank);
                  subrecord.skip_bytes(3);
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
                     subrecord.unchecked_read(this->stats.attributes.offsets.magicka);
                     subrecord.unchecked_read(this->stats.attributes.offsets.stamina);
                     subrecord.skip_bytes(2);                                   // 08 // Offsets here differ by form version
                     subrecord.unchecked_read(this->stats.level);               // 0A -> 08
                     subrecord.unchecked_read(this->stats.calc_min_level);      // 0C -> 0A
                     subrecord.unchecked_read(this->stats.calc_max_level);      // 0E -> 0C
                     subrecord.unchecked_read(this->stats.speed_mult);          // 10 -> 0E
                     subrecord.unchecked_read(this->stats.disposition);         // 12 -> 10
                     subrecord.unchecked_read(this->template_data.flags);       // 14 -> 12
                     subrecord.unchecked_read(this->stats.attributes.offsets.health); // 16 -> 14
                     subrecord.unchecked_read(this->stats.bleedout_threshold);  // 18 -> 16
                  }
                  break;
               }
               if (subrecord.is_in_bounds(0x18)) {
                  subrecord.unchecked_read(this->actor_flags);
                  subrecord.unchecked_read(this->stats.attributes.offsets.magicka);
                  subrecord.unchecked_read(this->stats.attributes.offsets.stamina);
                  subrecord.unchecked_read(this->stats.level);
                  subrecord.unchecked_read(this->stats.calc_min_level);
                  subrecord.unchecked_read(this->stats.calc_max_level);
                  subrecord.unchecked_read(this->stats.speed_mult);
                  subrecord.unchecked_read(this->stats.disposition);
                  subrecord.unchecked_read(this->template_data.flags);
                  subrecord.unchecked_read(this->stats.attributes.offsets.health);
                  subrecord.unchecked_read(this->stats.bleedout_threshold);
               }
               break;
            case 'TPLT':
               if (subrecord.read(this->template_data.actor)) {
                  intfc.warn_if_ref_is_wrong_type(this->template_data.actor, std::array{ form_type::actor_base, form_type::leveled_character }, subrecord.signature());
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
                     this->ai.aggro.attack      = val / 4;
                  }
                  this->ai.aggression = (ActorBase::aggression)aggression;
                  this->ai.assistance = (ActorBase::assistance)assistance;
                  this->ai.confidence = (ActorBase::confidence)confidence;
               }
               break;
            case 'ANAM':
               if (subrecord.read(this->far_away.model)) {
                  intfc.warn_if_ref_is_wrong_type(this->far_away.model, form_type::armor, subrecord.signature());
               }
               break;
            case 'CNAM':
               if (subrecord.read(this->stats.combat_class)) {
                  intfc.warn_if_ref_is_wrong_type(this->stats.combat_class, form_type::combat_class, subrecord.signature());
               }
               break;
            case 'DNAM':
               if (subrecord.is_in_bounds(0x34)) {
                  for (auto& byte : this->stats.skills.calculated.list)
                     subrecord.unchecked_read(byte);
                  for (auto& byte : this->stats.skills.offsets.list)
                     subrecord.unchecked_read(byte);
                  subrecord.unchecked_read(this->stats.attributes.calculated.health);
                  subrecord.unchecked_read(this->stats.attributes.calculated.magicka);
                  subrecord.unchecked_read(this->stats.attributes.calculated.stamina);
                  subrecord.skip_bytes(2);
                  subrecord.unchecked_read(this->far_away.distance);
                  subrecord.unchecked_read(this->geared_up_weapons);
                  subrecord.skip_bytes(3);
               }
               break;
            case 'FNAM':
               subrecord.read(espF8);
               break;
            case 'GNAM':
               if (subrecord.read(this->gift_filter)) {
                  intfc.warn_if_ref_is_wrong_type(this->gift_filter, form_type::formlist, subrecord.signature());
               }
               break;
            case 'INAM':
               if (subrecord.read(this->death_item)) {
                  intfc.warn_if_ref_is_wrong_type(this->death_item, form_type::leveled_item, subrecord.signature());
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
                  intfc.warn_if_ref_is_wrong_type(this->race, form_type::race, subrecord.signature());
               }
               break;
            case 'SNAM':
               if (subrecord.is_in_bounds(5)) {
                  faction_membership item;
                  subrecord.unchecked_read(item.faction);
                  subrecord.unchecked_read(item.rank);
                  intfc.warn_if_ref_is_wrong_type(item.faction, form_type::faction, subrecord.signature());

                  bool found = false;
                  for (auto& prior : this->faction_memberships) {
                     if (prior.faction == item.faction) {
                        found = true;
                        prior.rank = item.rank;
                        break;
                     }
                  }
                  if (!found) {
                     this->faction_memberships.push_back(item);
                  }
               }
               break;
            case 'WNAM':
               if (subrecord.read(this->skin)) {
                  intfc.warn_if_ref_is_wrong_type(this->skin, form_type::armor, subrecord.signature());
               }
               break;
            case 'ZNAM':
               if (subrecord.read(this->stats.combat_style)) {
                  intfc.warn_if_ref_is_wrong_type(this->stats.combat_style, form_type::combat_style, subrecord.signature());
               }
               break;
            case 'CRIF':
               if (subrecord.read(this->crime_faction)) {
                  intfc.warn_if_ref_is_wrong_type(this->crime_faction, form_type::faction, subrecord.signature());
               }
               break;
            case 'DOFT':
               if (subrecord.read(this->outfits.normal)) {
                  intfc.warn_if_ref_is_wrong_type(this->outfits.normal, form_type::outfit, subrecord.signature());
               }
               break;
            case 'SOFT':
               if (subrecord.read(this->outfits.sleeping)) {
                  intfc.warn_if_ref_is_wrong_type(this->outfits.sleeping, form_type::outfit, subrecord.signature());
               }
               break;
            case 'DPLT':
               if (subrecord.read(this->ai.default_package_list)) {
                  intfc.warn_if_ref_is_wrong_type(this->ai.default_package_list, form_type::formlist, subrecord.signature());
               }
               break;
            case 'FTST':
               if (auto& dst = this->facegen.complexion; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::texture_set, subrecord.signature());
               }
               break;
            case 'HEAD': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'ENAM': // found via disassembly; identical to PNAM
            [[fallthrough]];
            case 'PNAM':
               if (subrecord.read(form_id))
                  this->facegen.head_parts.push_back(form_id);
               break;
            case 'PKID':
               if (subrecord.read(form_id)) {
                  this->ai.package_list.push_back(form_id);
                  intfc.warn_if_ref_is_wrong_type(form_id, form_type::package, subrecord.signature());
               }
               break;
            case 'HCLF':
               if (auto& dst = this->facegen.hair_color; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::color, subrecord.signature());
               }
               break;
            case 'VTCK':
               if (subrecord.read(this->voicetype)) {
                  intfc.warn_if_ref_is_wrong_type(this->voicetype, form_type::voicetype, subrecord.signature());
               }
               break;
            case 'NAM0':
            case 'NAM1':
            case 'NAM2':
            case 'NAM3':
               if (esp1B == 0)
                  esp1B = 1;
               break;
            case 'NAM5':
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
               {
                  auto& dst = this->facegen.morphs.sliders;
                  using dst_type = std::decay_t<decltype(dst)>;

                  if (subrecord.is_in_bounds(sizeof(dst_type::value_type) * dst.size())) {
                     //
                     // If all of these floats are 0, then the TESNPC instance doesn't even bother 
                     // allocating storage for them.
                     //
                     for (auto& f : dst)
                        subrecord.unchecked_read(f);
                  }
               }
               break;
            case 'NAMA':
               {
                  auto& dst = this->facegen.morphs.indices;
                  using dst_type = std::decay_t<decltype(dst)>;

                  if (subrecord.is_in_bounds(sizeof(dst_type::value_type) * dst.size())) {
                     for (auto& f : dst)
                        subrecord.unchecked_read(f);
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
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
      form_id_t template_actor; // TPLT // Note: requires special use info flag: use_info_entry::flag::template_actor
      form_id_t voicetype;
      form_id_t skin; // WNAM
      struct {
         form_id_t spectator;      // SPOR
         form_id_t observe_corpse; // OCOR
         form_id_t guard_warn;     // GWOR
         form_id_t combat;         // ECOR
      } package_override_lists;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);
      structs::actor_creature_sounds::use_info_builder creature_sounds_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
               //
            #pragma region Components
            case components::attack_data::subrecord_signature_race:
            case components::attack_data::subrecord_signature_data:
            case components::attack_data::subrecord_signature_event:
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
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case components::spell_list::subrecord_signature_count:
            case components::spell_list::subrecord_signature_entry:
               components::spell_list::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            #pragma endregion
            case structs::actor_creature_sounds::subrecord_signature_inherit:
            case structs::actor_creature_sounds::subrecord_signature_sound_chance:
            case structs::actor_creature_sounds::subrecord_signature_sound_form:
            case structs::actor_creature_sounds::subrecord_signature_sound_start:
               structs::actor_creature_sounds::generate_use_info(subrecord, creature_sounds_uib);
               break;
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
               break;
            case 'PRKR': // perk
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
               subrecord.read(skin);
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
      destruction_uib.done();
      creature_sounds_uib.done();
      uib.add_outbound_reference(combat_class);
      uib.add_outbound_reference(combat_style);
      uib.add_outbound_reference(crime_faction);
      uib.add_outbound_reference(death_item);
      uib.add_outbound_reference(default_package_list);
      uib.add_outbound_reference(face_texture_set);
      uib.add_outbound_reference(far_away_model);
      uib.add_outbound_reference(gift_filter);
      uib.add_outbound_reference(outfits.normal);
      uib.add_outbound_reference(outfits.sleeping);
      uib.add_outbound_reference(race);
      uib.add_outbound_reference(template_actor, use_info::entry_flags::actor_base::template_actor);
      uib.add_outbound_reference(voicetype);
      uib.add_outbound_reference(skin);
      uib.add_outbound_reference(package_override_lists.spectator);
      uib.add_outbound_reference(package_override_lists.observe_corpse);
      uib.add_outbound_reference(package_override_lists.guard_warn);
      uib.add_outbound_reference(package_override_lists.combat);
   }
   void ActorBase::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ActorBase*)out;
      
      // components
      copy->attack_data.clone_from(this->attack_data, *copy);
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
      copy->inventory.clone_from(this->inventory, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      copy->spells.clone_from(this->spells, *copy);

      copy->name        = this->name;
      copy->short_name  = this->short_name;
      copy->actor_flags = this->actor_flags;
      copy->race.set(*copy, this->race);
      {  // ai
         copy_form_reference_list(*copy, copy->ai.package_list, this->ai.package_list);
         copy->ai.default_package_list.set(*copy, this->ai.default_package_list);
         {
            auto& src = this->ai.package_override_lists;
            auto& dst = copy->ai.package_override_lists;
            dst.combat.set(*copy, src.combat);
            dst.guard_warn.set(*copy, src.guard_warn);
            dst.observe_corpse.set(*copy, src.observe_corpse);
            dst.spectator.set(*copy, src.spectator);
         }
         copy->ai.aggression   = this->ai.aggression;
         copy->ai.confidence   = this->ai.confidence;
         copy->ai.energy_level = this->ai.energy_level;
         copy->ai.morality     = this->ai.morality;
         copy->ai.mood         = this->ai.mood;
         copy->ai.assistance   = this->ai.assistance;
         //
         copy->ai.aggro = this->ai.aggro;
      }
      {  // face
         auto& src = this->facegen;
         auto& dst = copy->facegen;
         dst.complexion.set(*copy, src.complexion);
         dst.hair_color.set(*copy, src.hair_color);
         copy_form_reference_list(*copy, dst.head_parts, src.head_parts);
         dst.morphs = src.morphs;
      }
      {
         auto& src = this->far_away;
         auto& dst = copy->far_away;
         dst.model.set(*copy, src.model);
         dst.distance = src.distance;
      }
      {
         auto& src = this->outfits;
         auto& dst = copy->outfits;
         dst.normal.set(*copy, src.normal);
         dst.sleeping.set(*copy, src.sleeping);
      }
      {
         auto& src = this->stats;
         auto& dst = copy->stats;
         dst.attributes = src.attributes;
         dst.skills     = src.skills;
         dst.level              = src.level;
         dst.calc_min_level     = src.calc_min_level;
         dst.calc_max_level     = src.calc_max_level;
         dst.speed_mult         = src.speed_mult;
         dst.disposition        = src.disposition;
         dst.bleedout_threshold = src.bleedout_threshold;
         dst.combat_class.set(*copy, src.combat_class);
         dst.combat_style.set(*copy, src.combat_style);
      }
      copy->template_data.actor.set(*copy, this->template_data.actor);
      copy->template_data.flags = this->template_data.flags;
      //
      copy->creature_sounds.clone_from(this->creature_sounds, *copy);
      copy->crime_faction.set(*copy, this->crime_faction);
      copy->death_item.set(*copy, this->death_item);
      {
         auto& src = this->faction_memberships;
         auto& dst = copy->faction_memberships;
         dst.resize(src.size());
         for (size_t i = 0; i < src.size(); ++i) {
            dst[i].faction.set(*copy, src[i].faction);
            dst[i].rank = src[i].rank;
         }
      }
      copy->geared_up_weapons = this->geared_up_weapons;
      copy->gift_filter.set(*copy, this->gift_filter);
      {
         auto& src = this->perks;
         auto& dst = copy->perks;
         dst.resize(src.size());
         for (size_t i = 0; i < src.size(); ++i) {
            dst[i].perk.set(*copy, src[i].perk);
            dst[i].rank = src[i].rank;
         }
      }
      copy->sound_level = this->sound_level;
      copy->texture_lighting = this->texture_lighting;
      copy->tint_layers = this->tint_layers;
      copy->voicetype.set(*copy, this->voicetype);
      copy->skin.set(*copy, this->skin);
      copy->height = this->height;
      copy->weight = this->weight;
   }
   void ActorBase::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      {
         auto& ACBS = record.open_next_subrecord('ACBS');
         if (record.version() < 0x1D) {
            ACBS.write(this->actor_flags);
            ACBS.write(this->stats.attributes.offsets.magicka);
            ACBS.write(this->stats.attributes.offsets.stamina);
            ACBS.skip_bytes(2);                                   // 08
            ACBS.write(this->stats.level);          // 0A -> 08
            ACBS.write(this->stats.calc_min_level); // 0C -> 0A
            ACBS.write(this->stats.calc_max_level); // 0E -> 0C
            ACBS.write(this->stats.speed_mult);     // 10 -> 0E
            ACBS.write(this->stats.disposition);    // 12 -> 10
            ACBS.write(this->template_data.flags); // 14 -> 12
            ACBS.write(this->stats.attributes.offsets.health);  // 16 -> 14
            ACBS.write(this->stats.bleedout_threshold); // 18 -> 16
         } else {
            ACBS.write(this->actor_flags);
            ACBS.write(this->stats.attributes.offsets.magicka);
            ACBS.write(this->stats.attributes.offsets.stamina);
            ACBS.write(this->stats.level);
            ACBS.write(this->stats.calc_min_level);
            ACBS.write(this->stats.calc_max_level);
            ACBS.write(this->stats.speed_mult);
            ACBS.write(this->stats.disposition);
            ACBS.write(this->template_data.flags);
            ACBS.write(this->stats.attributes.offsets.health);
            ACBS.write(this->stats.bleedout_threshold);
         }
         ACBS.close();
      }
      for (const auto& entry : this->faction_memberships) {
         auto& SNAM = record.open_next_subrecord('SNAM');
         SNAM.write(entry.faction);
         SNAM.write(entry.rank);
         SNAM.close();
      }
      record.write_formID_subrecord('INAM', this->death_item, true);
      record.write_formID_subrecord('VTCK', this->voicetype, true);
      record.write_formID_subrecord('TPLT', this->template_data.actor, true);
      record.write_formID_subrecord('RNAM', this->race);
      this->spells.save(record, intfc);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('WNAM', this->skin, true);
      record.write_formID_subrecord('ANAM', this->far_away.model, true);
      this->attack_data.save(record, intfc);
      record.write_formID_subrecord('SPOR', this->ai.package_override_lists.spectator, true);
      record.write_formID_subrecord('OCOR', this->ai.package_override_lists.observe_corpse, true);
      record.write_formID_subrecord('GWOR', this->ai.package_override_lists.guard_warn, true);
      record.write_formID_subrecord('ECOR', this->ai.package_override_lists.combat, true);
      if (!this->perks.empty()) {
         {
            auto& PRKZ = record.open_next_subrecord('PRKZ');
            PRKZ.write((uint32_t)this->perks.size());
            PRKZ.close();
         }
         for (const auto& entry : this->perks) {
            auto& PRKR = record.open_next_subrecord('PRKR');
            PRKR.write(entry.perk);
            PRKR.write(entry.rank);
            PRKR.skip_bytes(3);
            PRKR.close();
         }
      }
      this->inventory.save(record, intfc);
      {
         auto& AIDT = record.open_next_subrecord('AIDT');
         AIDT.write(this->ai.aggression);        // 00
         AIDT.write(this->ai.confidence);        // 01
         AIDT.write(this->ai.energy_level);      // 02
         AIDT.write(this->ai.morality);          // 03
         AIDT.write(this->ai.mood);              // 04
         AIDT.write(this->ai.assistance);        // 05
         AIDT.write(this->ai.aggro.use_radius);  // 06
         AIDT.skip_bytes(1);                     // 07
         AIDT.write(this->ai.aggro.warn);        // 08
         AIDT.write(this->ai.aggro.warn_attack); // 0C
         AIDT.write(this->ai.aggro.attack);      // 10
         AIDT.close();
      }
      for (const auto& entry : this->ai.package_list) {
         record.write_formID_subrecord('PKID', entry, true);
      }
      this->keywords.save(record, intfc);
      record.write_formID_subrecord('CNAM', this->stats.combat_class);
      if (!this->name.empty()) {
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
      }
      if (!this->short_name.empty()) {
         auto& SHRT = record.open_next_subrecord('SHRT');
         SHRT.write(this->short_name);
         SHRT.close();
      }
      {  // marker subrecord (how janky.)
         auto& DATA = record.open_next_subrecord('DATA');
         DATA.close();
      }
      {
         auto& DNAM = record.open_next_subrecord('DNAM');
         for (auto& byte : this->stats.skills.calculated.list)
            DNAM.write(byte);
         for (auto& byte : this->stats.skills.offsets.list)
            DNAM.write(byte);
         DNAM.write(this->stats.attributes.calculated.health);
         DNAM.write(this->stats.attributes.calculated.magicka);
         DNAM.write(this->stats.attributes.calculated.stamina);
         DNAM.skip_bytes(2);
         DNAM.write(this->far_away.distance);
         DNAM.write(this->geared_up_weapons);
         DNAM.skip_bytes(3);
         DNAM.close();
      }
      for (const auto& entry : this->facegen.head_parts) {
         record.write_formID_subrecord('PNAM', entry, true);
      }
      record.write_formID_subrecord('HCLF', this->facegen.hair_color, true);
      record.write_formID_subrecord('ZNAM', this->stats.combat_style, true);
      record.write_formID_subrecord('GNAM', this->gift_filter, true);
      {
         //
         // A NAM5 subrecord, empty or with dummy bytes, can be found here, but the game doesn't 
         // seem to load it. The CK always serializes it as a two-byte value: 0x00FF.
         //
         uint16_t value = 0x00FF;

         auto& NAM5 = record.open_next_subrecord('NAM5');
         NAM5.write(value);
         NAM5.close();
      }
      {
         auto& NAM6 = record.open_next_subrecord('NAM6');
         NAM6.write(this->height);
         NAM6.close();
      }
      {
         auto& NAM7 = record.open_next_subrecord('NAM7');
         NAM7.write(this->weight);
         NAM7.close();
      }
      {
         auto& NAM8 = record.open_next_subrecord('NAM8');
         NAM8.write((uint32_t)this->sound_level);
         NAM8.close();
      }
      this->creature_sounds.save(record, intfc);
      record.write_formID_subrecord('DOFT', this->outfits.normal, true);
      record.write_formID_subrecord('SOFT', this->outfits.sleeping, true);
      record.write_formID_subrecord('DPLT', this->ai.default_package_list, true);
      record.write_formID_subrecord('CRIF', this->crime_faction, true);
      record.write_formID_subrecord('FTST', this->facegen.complexion, true);
      {
         auto& QNAM = record.open_next_subrecord('QNAM');
         QNAM.write(this->texture_lighting.r);
         QNAM.write(this->texture_lighting.g);
         QNAM.write(this->texture_lighting.b);
         QNAM.close();
      }
      if (!this->facegen.morphs.all_sliders_zeroed()) {
         auto& NAM9 = record.open_next_subrecord('NAM9');
         for (auto f : this->facegen.morphs.sliders)
            NAM9.write(f);
         NAM9.close();
      }
      if (!this->facegen.morphs.all_indexed_morphs_zeroed()) {
         auto& NAMA = record.open_next_subrecord('NAMA');
         for (auto v : this->facegen.morphs.indices)
            NAMA.write(v);
         NAMA.close();
      }
      for (const auto& entry : this->tint_layers) {
         {
            auto& TINI = record.open_next_subrecord('TINI');
            TINI.write(entry.index);
            TINI.close();
         }
         {
            auto& TINC = record.open_next_subrecord('TINC');
            TINC.write(entry.color.r);
            TINC.write(entry.color.g);
            TINC.write(entry.color.b);
            TINC.write(entry.color.unused);
            TINC.close();
         }
         {
            auto& TINV = record.open_next_subrecord('TINV');
            TINV.write(entry.interpolation);
            TINV.close();
         }
         {
            auto& TIAS = record.open_next_subrecord('TIAS');
            TIAS.write(entry.preset);
            TIAS.close();
         }
      }
   }
   void ActorBase::_sever_outbound_references_impl(form_stub& other) noexcept {
      // components
      this->attack_data.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->inventory.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->spells.sever_outbound_references_to(other, *this);

      this->race.clear_if(*this, other);
      {  // ai
         remove_form_from_reference_list(this->ai.package_list, other, *this);
         this->ai.default_package_list.clear_if(*this, other);
         {
            auto& dst = this->ai.package_override_lists;
            dst.combat.clear_if(*this, other);
            dst.guard_warn.clear_if(*this, other);
            dst.observe_corpse.clear_if(*this, other);
            dst.spectator.clear_if(*this, other);
         }
      }
      {  // face
         auto& dst = this->facegen;
         dst.complexion.clear_if(*this, other);
         dst.hair_color.clear_if(*this, other);
         remove_form_from_reference_list(dst.head_parts, other, *this);
      }
      this->far_away.model.clear_if(*this, other);
      {
         auto& dst = this->outfits;
         dst.normal.clear_if(*this, other);
         dst.sleeping.clear_if(*this, other);
      }
      {
         auto& dst = this->stats;
         dst.combat_class.clear_if(*this, other);
         dst.combat_style.clear_if(*this, other);
      }
      this->template_data.actor.clear_if(*this, other);
      //
      this->creature_sounds.sever_outbound_references_to(other, *this);
      this->crime_faction.clear_if(*this, other);
      this->death_item.clear_if(*this, other);
      {
         bool  any = false;
         auto& dst = this->faction_memberships;
         for (size_t i = 0; i < dst.size(); ++i) {
            dst[i].faction.clear_if(*this, other);
            if (dst[i].faction == nullptr)
               any = true;
         }
         if (any) {
            std::erase_if(dst, [](const auto& entry) -> bool { return entry.faction == nullptr; });
         }
      }
      this->gift_filter.clear_if(*this, other);
      {
         bool  any = false;
         auto& dst = this->perks;
         for (auto& entry : dst) {
            entry.perk.clear_if(*this, other);
            if (entry.perk == nullptr)
               any = true;
         }
         if (any) {
            std::erase_if(dst, [](const auto& entry) -> bool { return entry.perk == nullptr; });
         }
      }
      this->voicetype.clear_if(*this, other);
      this->skin.clear_if(*this, other);
   }
   void ActorBase::_clear_impl() noexcept {
      // components
      this->attack_data.clear(*this);
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->inventory.clear(*this);
      this->keywords.clear(*this);
      this->script_data.clear(*this);
      this->spells.clear(*this);

      this->name.reset();
      this->short_name.reset();
      this->actor_flags = 0;
      this->race.set(*this, nullptr);
      {  // ai
         clear_form_reference_list(this->ai.package_list, *this);
         this->ai.default_package_list.set(*this, nullptr);
         {
            auto& dst = this->ai.package_override_lists;
            dst.combat.set(*this, nullptr);
            dst.guard_warn.set(*this, nullptr);
            dst.observe_corpse.set(*this, nullptr);
            dst.spectator.set(*this, nullptr);
         }
         this->ai.aggression   = aggression::unaggressive;
         this->ai.confidence   = confidence::average;
         this->ai.energy_level = 50;
         this->ai.morality     = morality::no_crime;
         this->ai.mood         = mood::neutral;
         this->ai.assistance   = assistance::helps_friends_and_allies;
         //
         this->ai.aggro = {};
         this->ai.aggro.warn = 0;
         this->ai.aggro.warn_attack = 0;
         this->ai.aggro.attack = 0;
      }
      {  // face
         auto& dst = this->facegen;
         dst.complexion.set(*this, nullptr);
         dst.hair_color.set(*this, nullptr);
         clear_form_reference_list(dst.head_parts, *this);
         dst.morphs = {};
      }
      this->far_away.model.set(*this, nullptr);
      this->far_away.distance = 0;
      {
         auto& dst = this->outfits;
         dst.normal.set(*this, nullptr);
         dst.sleeping.set(*this, nullptr);
      }
      {
         auto& dst = this->stats;
         dst.level          = 1;
         dst.calc_min_level = 0;
         dst.calc_max_level = 0;
         dst.speed_mult     = 0;
         dst.disposition    = 0;
         dst.bleedout_threshold = 0;
         dst.combat_class.set(*this, nullptr);
         dst.combat_style.set(*this, nullptr);
         //
         dst.attributes = {};
         dst.skills     = {};
         for (auto& skill : dst.skills.calculated.list)
            skill = 5; // TODO: use GMST iAVDSkillStart
      }
      this->template_data.actor.set(*this, nullptr);
      this->template_data.flags = 0;
      //
      this->creature_sounds.clear(*this);
      this->crime_faction.set(*this, nullptr);
      this->death_item.set(*this, nullptr);
      {
         auto& dst = this->faction_memberships;
         for (size_t i = 0; i < dst.size(); ++i) {
            dst[i].faction.set(*this, nullptr);
         }
         dst.clear();
      }
      this->geared_up_weapons = 0;
      this->gift_filter.set(*this, nullptr);
      {
         auto& dst = this->perks;
         for (size_t i = 0; i < dst.size(); ++i) {
            dst[i].perk.set(*this, nullptr);
         }
         dst.clear();
      }
      this->sound_level = 0;
      this->texture_lighting = {};
      this->tint_layers.clear();
      this->voicetype.set(*this, nullptr);
      this->skin.set(*this, nullptr);
      this->height = 1.0;
      this->weight = 0.0;
   }
}