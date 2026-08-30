#pragma once
#include <optional>
#include "./Form.h"
#include "./_common.h"
#include "./components/papyrus.h"

namespace dovah::loaded_forms {
   class CombatStyle : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::combat_style;
         CombatStyle(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               allow_dual_wielding = 0x00080000,
            };
         };

         struct flag {
            enum type : uint32_t {
               dueling = 0x01,
               flanking = 0x02,
               allow_dual_wielding = 0x04,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct CSMD {
            float unk00;
            float unk04;
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD

         flags_t flags = 0;
         struct {
            float offensive_mult = 1;
            float defensive_mult = 1;
            float group_offensive_mult = 1;
            struct {
               float melee   = 1;
               float magic   = 1;
               float ranged  = 1;
               float shout   = 1;
               float unarmed = 1;
               float staff   = 1;
            } equipment_score_mults;
            float avoid_threat_chance = 1;
         } general; // CSGD
         struct {
            float attack_staggered_mult = 1;
            float power_attack_staggered_mult = 1;
            float power_attack_blocking_mult = 1;
            float bash_mult = 1;
            float bash_recoil_mult = 1;
            float bash_attack_mult = 1;
            float bash_power_attack_mult = 1;
            float special_attack_mult = 1;
         } melee; // CSME
         struct {
            float circle_mult = 1;
            float fallback_mult = 1;
            float flank_distance = 0;
            float stalk_time = 0;
         } close_range; // CSCR
         struct {
            float strafe_mult = 1;
         } long_range; // CSLR
         struct {
            struct {
               float chance = 1; // CSFL+0x04
            } divebomb;
            struct {
               float chance = 1; // CSFL+0x1C
            } flying_attack;
            struct {
               float chance = 1; // CSFL+0x08
               float time   = 0; // CSFL+0x10
            } ground_attack;
            struct {
               float time   = 0; // CSFL+0x0C
               float chance = 1; // CSFL+0x00
            } hover;
            struct {
               float chance = 1; // CSFL+0x14
               float time   = 0; // CSFL+0x18
            } perch_attack;
         } flight; // CSFL
         std::optional<CSMD> csmd;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}