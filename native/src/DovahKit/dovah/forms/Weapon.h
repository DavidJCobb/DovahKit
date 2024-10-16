#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/enchantable.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"
#include "../data/skills.h"
#include "../data/weapon_type.h"

namespace dovah::loaded_forms {
   class Weapon : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::weapon;
         Weapon(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable = 0x00000004,
            };
         };

         enum class legacy_attack_animation : uint8_t {
            AttackLeft = 26,
            AttackRight = 32,
            Attack3 = 38,
            Attack4 = 44,
            Attack5 = 50,
            Attack6 = 56,
            Attack7 = 62,
            Attack8 = 68,
            AttackLoop = 74,
            AttackSpin = 80,
            AttackSpin2 = 86,
            PlaceMine = 97,
            PlaceMine2 = 103,
            AttackThrow = 109,
            AttackThrow2 = 115,
            AttackThrow3 = 121,
            AttackThrow4 = 127,
            AttackThrow5 = 133,
            DEFAULT = 255,
         };

         enum class hit_gore : uint32_t {
            normal,
            dismember_only,
            explode_only,
            no_dismember_or_explode,
         };

         enum class rumble_pattern : uint32_t {
            constant,
            periodic_square,
            periodic_triangle,
            periodic_sawtooth,
         };

         components::object_bounds bounds; // OBND
         components::model_ts      model;  // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::enchantable  enchantable; // EITM, EAMT
         components::keyword_list keywords;    // KSIZ, KWDA
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         form_reference_t template_weapon; // CNAM
         //
         form_reference_t equip_type; // ETYP
         form_reference_t first_person_model; // WNAM // a STAT form
         form_reference_t impact_data_set; // INAM
         struct {
            form_reference_t attack;      // SNAM
            form_reference_t attack_2D;   // XNAM
            form_reference_t attack_loop; // NAM7
            form_reference_t attack_fail; // TNAM
            form_reference_t idle;        // UNAM
            form_reference_t equip;       // NAM9
            form_reference_t unequip;     // NAM8
         } sounds;
         //
         weapon_type type   = weapon_type::one_hand_dagger; // DNAM+0x00
         uint16_t    damage = 0; // DATA+0x08
         float       speed  = 1; // DNAM+0x04
         float       reach  = 0; // DNAM+0x08
         float       unk_dnam_10 = 0; // DNAM+0x10
         float       unk_dnam_30 = 0; // DNAM+0x30
         float       unk_dnam_40 = 0; // DNAM+0x40
         float       ironsight_fov = 0; // DNAM+0x14
         uint8_t     base_vats_hit_chance = 0; // DNAM+0x1C
         uint8_t     projectile_count = 1; // DNAM+0x1E
         hit_gore    hit_gore_behavior = hit_gore::normal; // DNAM+0x28
         dovah::skill skill = dovah::skill::one_handed;
         int32_t     resist_av = -1;
         float       stagger = 0;
         detection_loudness loudness = detection_loudness::normal; // VNAM
         struct {
            float minimum = 0; // DNAM+0x20
            float maximum = 0; // DNAM+0x24
         } ai_ranges;
         struct {
            float attack_mult = 1;
            legacy_attack_animation legacy_anim = legacy_attack_animation::DEFAULT; // DNAM+0x1D
         } animation;
         struct {
            form_reference_t alternate_material;
            form_reference_t impact_data_set;
         } block_bash;
         struct {
            uint16_t added_damage = 0;
            float    chance_mult  = 1.0F;
            form_reference_t spell_to_apply;
            bool apply_spell_only_on_target_death = false;
         } crit_data; // CRDT
         struct {
            std::string node;
            uint8_t     actor_value = 0; // DNAM+0x1F
         } embedded;
         struct {
            //
            // Flags are divided into the following locations:
            // 
            //  - uint16_t DNAM+0x0C: Weapon Flags A
            //  - uint32_t DNAM+0x2C: Weapon Flags B
            // 
            bool automatic                      = false; // flags A, bit (1 << 1)
            bool bound_weapon                   = false; // flags B, bit (1 << 13)
            bool burst_shot                     = false; // flags B, bit (1 << 9)
            bool cant_drop                      = false; // flags A, bit (1 << 3)
            bool embedded                       = false; // flags A, bit (1 << 5)
            bool fixed_ai_range                 = false; // flags B, bit (1 << 5)
            bool has_scope                      = false; // flags A, bit (1 << 2)
            bool hide_backpack                  = false; // flags A, bit (1 << 4)
            bool ignores_normal_weapon_resist   = false; // flags A, bit (1 << 0)
            bool long_bursts                    = false; // flags B, bit (1 << 11)
            bool minor_crime                    = false; // flags B, bit (1 << 4)
            bool never_jams_after_reload        = false; // flags B, bit (1 << 2)
            bool no_first_person_ironsight_anim = false; // flags A, bit (1 << 6)
            bool no_third_person_ironsight_anim = false; // flags B, bit (1 << 8)
            bool non_hostile                    = false; // flags B, bit (1 << 12)
            bool non_playable                   = false; // flags A, bit (1 << 7)
            bool not_used_in_normal_combat      = false; // flags B, bit (1 << 6)
            bool npcs_use_ammo                  = false; // flags B, bit (1 << 1)
            bool player_only                    = false; // flags B, bit (1 << 0)
            bool rumble_alternate               = false; // flags B, bit (1 << 10)
            //
            bool unk_flag_b_3 = false; // flags B, bit (1 << 3)
            bool unk_flag_b_7 = false; // flags B, bit (1 << 7)
         } flags;
         struct {
            float   weight = 0; // DATA+0x04
            int32_t value  = 0; // DATA+0x00
            struct {
               std::string inventory; // ICON
               std::string message;   // MICO
            } icons;
            struct {
               form_reference_t take; // YNAM // sound when picked up
               form_reference_t drop; // ZNAM // sound when dropped
            } sounds;
         } item_data;
         struct {
            float left_motor  = 0;
            float right_motor = 0;
            float duration    = 0;
            rumble_pattern pattern = rumble_pattern::constant;
         } rumble;
         components::model scope_model; // MOD3, MO3T, MO3S
         form_reference_t scope_shader; // EFSD

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}