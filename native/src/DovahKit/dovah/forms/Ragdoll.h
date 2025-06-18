#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Ragdoll : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::ragdoll;
         Ragdoll(const constructor_params& c) : Form(form_type, c) {};

      public:
         components::model model;
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct { // sizeof == 0xE
            uint16_t dynamic_bone_count = 0; // 00 // number of values in RAFB. DATA must precede RAFB, because the array is init'd via [the equivalent of] std::vector::resize
            uint16_t unk02 = 0; // size of some sort of array in the form's run-time data
            uint16_t unk04 = 0; // size of some sort of array in the form's run-time data
            uint16_t unk06 = 0; // size of some sort of array in the form's run-time data
            bool     feedback = 0;
            bool     foot_ik = 0;
            bool     look_ik = 0;
            bool     grab_ik = 0;
            bool     pose_matching = 0;
            uint8_t  unk0D = 0;
         } data; // DATA
         struct { // sizeof == 0x3C
            float dynamic_keyframe_blend_amount = 0.9; // 00
            struct {
               float hierarchy    = 0.8; // 04
               float position     = 0.4; // 08
               float velocity     = 0.8; // 0C
               float acceleration = 0.1; // 10
               float snap         = 0.3; // 14
            } gain;
            float velocity_damping = 0; // 18
            struct {
               struct {
                  float linear  = 50; // 1C
                  float angular = 50; // 20
               } velocity;
               struct {
                  float linear  = 25; // 24
                  float angular = 25; // 28
               } distance;
            } snap_max;
            struct {
               float   linear     = 50;    // 2C
               float   angular    = 50;    // 30
               int32_t projectile = 10000; // 34 // stored as fixed-point, to two decimal places
               int32_t melee      = 30000; // 38 // stored as fixed-point, to two decimal places
            } max_velocity;
         } feedback_data; // RAFD
         struct { // sizeof == 0x18
            std::array<uint16_t, 3> bones = { 0xFFFF, 0, 0 }; // 00, 02, 04
            bool     disable_on_move = 0; // 06
            // padding
            float    motors_strength            = 0;   // 08
            float    pose_activation_delay_time = 0;   // 0C
            float    match_error_allowance      = 0.1; // 10
            float    displacement_to_disable    = 0;   // 14
         } pose_matching_data; // RAPS
         std::vector<uint16_t> feedback_dynamic_bones; // RAFB
         //
         std::string      death_pose;     // ANAM
         form_reference_t body_part_data; // TNAM
         form_reference_t preview_actor;  // XNAM
         uint32_t         version = 1;    // NVER

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