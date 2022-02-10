#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "structs/color_dword.h"
#include "components/attack_data.h"
#include "components/bounds.h"
#include "components/container.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ActorBase : public Form {
      public:
         static constexpr form_type_t form_type = form_type::actor_base;
         ActorBase(const constructor_params& c) : Form(form_type, c) {};

         struct actor_flag {
            enum type : uint32_t {
               female                      = 0x00000001,
               essential                   = 0x00000002,
               is_chargen_preset           = 0x00000004,
               respawn                     = 0x00000008,
               auto_calc_stats             = 0x00000010,
               unique                      = 0x00000020,
               doesnt_affect_stealth_meter = 0x00000040,
               pc_level_mult               = 0x00000080,
               use_template                = 0x00000100, // unconfirmed
               //
               is_protected                = 0x00000800,
               //
               summonable                  = 0x00004000,
               //
               doesnt_bleed                = 0x00010000,
               //
               bleedout_override           = 0x00040000,
               opposite_gender_animations  = 0x00080000,
               simple_actor                = 0x00100000,
               looped_script               = 0x00200000, // unconfirmed
               looped_audio                = 0x10000000, // unconfirmed
               ghost                       = 0x20000000,
               //
               invulnerable                = 0x80000000
            };
         };
         using actor_flags_t = std::underlying_type_t<actor_flag::type>;

         enum class aggression : uint8_t {
            unaggressive    = 0, // Does not attack unless provoked.
            aggressive      = 1, // Attacks enemies on sight.
            very_aggressive = 2, // Attacks enemies and neutrals on sight.
            frenzied        = 3, // Attacks everything on sight.
         };

         enum class assistance : uint8_t { // Controls whether the actor will assist other actors in combat.
            helps_nobody = 0,
            helps_allies = 1,
            helps_friends_and_allies = 2,
         };

         enum class confidence : uint8_t {
            cowardly  = 0, // Never engages in combat. Always avoids or flees from threats.
            cautious  = 1, // Avoids or flees from any threat it doesn't know for certain it can beat.
            average   = 2, // Avoids or flees from threats if outmatched.
            brave     = 3, // Avoids or flees from threats only if severely outmatched.
            foolhardy = 4, // Never avoids or flees from anything.
         };

         enum class creature_sound_type : uint32_t {
            idle           = 0,
            aware          = 1,
            attack         = 2,
            hit            = 3,
            death          = 4,
            weapon         = 5,
            movement_loop  = 6,
            conscious_loop = 7,
         };
         struct creature_sound {
            creature_sound_type type   = creature_sound_type::idle;
            form_reference_t    sound;
            uint8_t             chance = 100; // chance to play (percentage)
         };

         struct faction_membership {
            form_reference_t faction;
            int8_t rank = 0;
         };

         enum class mood : uint8_t {
            neutral   = 0,
            angry     = 1,
            fear      = 2,
            happy     = 3,
            sad       = 4,
            surprised = 5,
            puzzled   = 6,
            disgusted = 7,
         };

         enum class morality : uint8_t { // controls what orders the actor is willing to obey from the player
            any_crime                = 0, // the actor is willing to commit any crime
            violence_against_enemies = 1, // the actor is willing to commit violent crimes against enemies
            property_crimes_only     = 2, // the actor will refuse to commit violent crimes
            no_crime                 = 3, // the actor will refuse to commit any crime
         };

         struct template_flag {
            enum type : uint16_t {
               use_traits      = 0x0001,
               use_stats       = 0x0002,
               use_factions    = 0x0004,
               use_spells      = 0x0008,
               use_ai_data     = 0x0010,
               use_ai_packages = 0x0020,
               use_animations  = 0x0040, // and model?
               use_base_data   = 0x0080,
               use_inventory   = 0x0100,
               use_scripts     = 0x0200,
               use_package_overrides = 0x0400, // "Use Def Pack List" in the CK
               use_attack_data = 0x0800,
               use_keywords    = 0x1000,
            };
         };
         using template_flags_t = std::underlying_type_t<template_flag::type>;

         union skill_byte_list {
            struct {
               uint8_t one_handed;
               uint8_t two_handed;
               uint8_t archery;
               uint8_t block;
               uint8_t smithing;    // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
               uint8_t heavy_armor;
               uint8_t light_armor;
               uint8_t pickpocket;  // only used by NPCs to scale how difficult it is for the player to pickpocket them; NPCs cannot pickpocket others.
               uint8_t lockpicking; // not used by NPCs; they only lockpick when commanded to, and in that case will always succeed.
               uint8_t sneak;
               uint8_t alchemy;     // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
               uint8_t speech;      // not used by NPCs.
               uint8_t alteration;
               uint8_t conjuration;
               uint8_t destruction;
               uint8_t illusion;
               uint8_t restoration;
               uint8_t enchanting;  // NPCs never craft on their own, though it's not impossible that a mod might read this and use it.
            };
            std::array<uint8_t, 18> list = {};
         };

         struct tint_layer {
            uint16_t index = 0;
            color_t  color;      // the game skips loading this if (actor_flag::is_chargen_preset) is set and if INI setting [General]bUseFaceGenPreprocessedHeads is true
            uint32_t interpolation = 0; // fixed-point: float times 100. 
            uint16_t preset = 0; // the game skips loading this if (actor_flag::is_chargen_preset) is set and if INI setting [General]bUseFaceGenPreprocessedHeads is true
         };

         components::attack_data             attack_data; // ATKR, ATKD+ATKE
         components::object_bounds           bounds; // OBND
         components::destruction_stage_data  destruction_data; // DEST
         components::container_data          inventory;
         components::keyword_list            keywords; // KSIZ, KWDA
         components::papyrus_attachment_data script_data; // VMAD
         localized_string name;            // FULL
         localized_string short_name;      // SHRT
         actor_flags_t    actor_flags = 0; // ACBS+0x00
         form_reference_t race;            // RNAM
         struct {
            std::vector<form_reference_t> package_list; // PKID[]
            form_reference_t default_package_list; // DPLT
            struct {
               form_reference_t spectator;      // SPOR
               form_reference_t observe_corpse; // OCOR
               form_reference_t guard_warn;     // GWOR
               form_reference_t combat;         // ECOR
            } package_override_lists; // same structure as on QUST
            //
            // Data below is from AIDT, the TESAIDataForm content.
            //
            aggression aggression   = aggression::unaggressive;
            confidence confidence   = confidence::average;
            uint8_t    energy_level = 0;
            morality   morality     = morality::any_crime;
            mood       mood         = mood::neutral;
            assistance assistance   = assistance::helps_friends_and_allies;
            struct {
               bool     use_radius = false;
               uint32_t warn;        // warn targets while they're in this radius                        // at run-time, this is clamped to 0xFFFF and stored as a uint16_t
               uint32_t warn_attack; // warn targets while they're in this radius; attack if they remain // at run-time, this is clamped to 0xFFFF and stored as a uint16_t
               uint32_t attack;      // attack targets that enter this radius                            // at run-time, this is clamped to 0xFFFF and stored as a uint16_t
            } aggro;
         } ai;
         struct {
            struct {
               struct {
                  float length;
                  float height;
               } nose;
               struct {
                  float height;
                  float width;
                  float depth;
               } jaw;
               struct {
                  float height;
                  float depth;
               } cheeks;
               struct {
                  float height;
                  float width;
                  float depth; // read AFTER chin
               } eyes;
               struct {
                  float height;
                  float width;
                  float depth;
               } brows;
               struct {
                  float height;
                  float depth;
               } lips;
               struct {
                  float width;
                  float height;
                  float depth;
               } chin;
               float unknown;
            } morphs; // NAM9 // The game doesn't even bother to store this if they're all zero.
            struct {
               uint32_t nose;
               uint32_t unknown;
               uint32_t eyes;
               uint32_t mouth;
            } parts; // NAMA
            form_reference_t texture_set; // FTST
         } face;
         struct {
            form_reference_t model; // ANAM // an Armor
            float distance = 0.0F; // DNAM+0x2C // distance at which the far-away model is applied
         } far_away;
         struct {
            std::vector<form_reference_t> hair_colors;  // HCLF[]
            std::vector<form_reference_t> head_parts;   // HEAD[] and/or PNAM[] and/or ENAM[]
         } head;
         struct {
            form_reference_t normal; // default
            form_reference_t sleeping;
         } outfits;
         struct {
            struct {
               skill_byte_list skills; // DNAM+0x00
               uint16_t health;  // DNAM+0x24
               uint16_t magicka; // DNAM+0x26
               uint16_t stamina; // DNAM+0x28
            } base;
            struct {
               skill_byte_list skills; // DNAM+0x18
               int16_t health  = 0; // ABCS+0x14
               int16_t magicka = 0; // ABCS+0x04
               int16_t stamina = 0; // ABCS+0x06
            } offsets;
            int16_t  level;              // ABCS+0x08 // this is a multiplier (fixed-point; convert to a float by dividing by 1000) if the PC Level Mult flag is set
            int16_t  calc_min_level = 0; // ABCS+0x0A // applies if the PC Level Mult flag is set
            int16_t  calc_max_level = 0; // ABCS+0x0C // applies if the PC Level Mult flag is set
            int16_t  speed_mult     = 0; // ABCS+0x0E
            int16_t  disposition    = 0; // ABCS+0x10
            uint16_t bleedout_threshold; // ABCS+0x16 // override the Bleedout Default on the actor's CLAS form, if the Bleedout Override actor flag is set
            form_reference_t combat_class; // CNAM
            form_reference_t combat_style; // ZNAM
         } stats;
         struct {
            form_reference_t actor;     // TPLT
            template_flags_t flags = 0; // ABCS+0x12
         } template_data;
         //
         std::vector<creature_sound> creature_sounds;
         form_reference_t crime_faction; // CRIF
         form_reference_t death_item; // INAM
         std::vector<faction_membership> faction_memberships; // SNAM[]
         uint8_t geared_up_weapons = 0; // DNAM+0x30 // unused
         form_reference_t gift_filter; // GNAM // a FormList
         std::vector<form_reference_t> perks;
         uint8_t sound_level = 0;
         std::vector<form_reference_t> spells; // SPLO[SPCT], but SPCT is just a suggestion; the game will properly reallocate the list as needed
         struct {
            float r = 255.0F; // value in the range of [0.0F, 255.0F]... but then why the hell is it encoded as a float?
            float g = 255.0F;
            float b = 255.0F;
         } texture_lighting; // QNAM
         std::vector<tint_layer> tint_layers;
         form_reference_t voicetype; // VTCK
         form_reference_t worn_armor; // WNAM
         float height = 1.0F;
         float weight = 0.0F;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc); // TODO: FINISH ME
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
   };
}