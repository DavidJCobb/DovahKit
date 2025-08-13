#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/keyword_list.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"
#include "../data/magic_casting_type.h"
#include "../data/magic_delivery_type.h"
#include "../data/magic_effect_archetypes.h"
#include "../data/skills.h"

namespace dovah::loaded_forms {
   class MagicEffect : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::magic_effect;
         MagicEffect(const constructor_params& c) : Form(form_type, c) {};

         enum class effect_sound_type {
            draw_sheathe,
            charge,
            ready,
            release,
            concentration_cast_loop,
            on_hit,
         };

         struct effect_sound {
            effect_sound_type type = effect_sound_type::draw_sheathe;
            form_reference_t  descriptor; // SNDR
         };

         struct effect_flag {
            enum type : uint32_t {
               hostile = 0x00000001,
               recover = 0x00000002,
               detrimental = 0x00000004,
               snap_to_navmesh = 0x00000008,
               no_hit_event = 0x00000010,
               //
               dispel_with_keywords = 0x00000100,
               no_duration = 0x00000200,
               no_magnitude = 0x00000400,
               no_area = 0x00000800,
               fx_persist = 0x00001000,
               //
               gory_visuals = 0x00004000,
               hide_in_ui = 0x00008000,
               //
               no_recast = 0x00020000,
               //
               power_affects_magnitude = 0x00200000,
               power_affects_duration  = 0x00400000,
               unknown_24              = 0x01000000, // CK: if set, the effect doesn't contribute to total spell cost
               //
               painless = 0x04000000,
               no_hit_effect = 0x08000000,
               no_death_dispel = 0x10000000,
            };
         };
         using effect_flags_t = std::underlying_type_t<effect_flag::type>;

      public:
         components::condition_list conditions; // CTDA and friends
         components::papyrus_attachment_data script_data; // VMAD
         components::keyword_list keywords; // KSIZ, KWDA
         //
         localized_string name        = localized_string(localized_string_type::common); // FULL
         localized_string description = localized_string(localized_string_type::common); // DNAM
         //
         effect_flags_t         flags = 0; // DATA+0x00
         magic_effect_archetype archetype        = magic_effect_archetype::value_modifier; // DATA+0x44
         magic_casting_type     casting_type     = magic_casting_type::constant_effect; // DATA+0x54
         magic_delivery_type    delivery_type    = magic_delivery_type::self; // DATA+0x58
         int32_t                magic_skill      = -1; // DATA+0x10 (AV index)
         uint32_t               min_skill_level  = 0; // DATA+0x2C
         float                  base_cost        = 0.0F; // DATA+0x04
         float                  skill_usage_mult = 1.0F; // DATA+0x6C
         int32_t                resist_av        = -1; // DATA+0x14
         form_reference_t       equip_ability; // DATA+0x84
         form_reference_t       perk_to_apply; // DATA+0x8C
         form_reference_t       menu_display_object;
         std::vector<form_reference_t> counter_effects; // ESCE[] // values are MGEF
         struct {
            float score    = 0.0F; // DATA+0x94
            float cooldown = 0.0F; // DATA+0x98
         } ai_params;
         struct {
            std::array<int32_t, 2> actor_value_indices = { -1, -1 }; // DATA+0x48, DATA+0x5C
            form_reference_t       form;
            float                  second_av_weight = 0; // DATA+0x40
         } associated_items;
         struct {
            std::vector<effect_sound> sounds; // SNDD
            detection_loudness casting_loudness = detection_loudness::normal; // DATA+0x90 // uint32_t
         } audio;
         struct {
            form_reference_t dual_cast_data; // DATA+0x70
            float scale = 1.0F; // DATA+0x74
         } dual_casting;
         struct {
            uint32_t area        = 0; // DATA+0x30
            float    casting_time = 0; // DATA+0x34
         } spellmaking;
         struct {
            float weight   = 0; // DATA+0x20
            float curve    = 0; // DATA+0x38
            float duration = 0; // DATA+0x3C
         } taper;
         struct {
            struct {
               form_reference_t art;   // DATA+0x60
               form_reference_t light; // DATA+0x1C
            } casting;
            struct {
               form_reference_t art;    // DATA+0x78
               form_reference_t shader; // DATA+0x28
            } enchant;
            struct {
               form_reference_t effect_art; // DATA+0x64
               form_reference_t shader;     // DATA+0x24
            } hit;
            form_reference_t explosion; // DATA+0x50
            form_reference_t imagespace_modifier; // DATA+0x88
            form_reference_t impact_data_set; // DATA+0x68
            form_reference_t projectile; // DATA+0x4C
            form_reference_t unk_visual_effect_a; // DATA+0x7C
            form_reference_t unk_visual_effect_b; // DATA+0x80
         } vfx;

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