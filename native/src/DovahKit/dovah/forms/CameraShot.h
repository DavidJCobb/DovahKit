#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class CameraShot : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::camera_shot;
         CameraShot(const constructor_params& c) : Form(form_type, c) {};

         enum class camera_action : uint32_t {
            shoot,
            fly,
            hit,
            zoom,
         };
         enum class camera_subject : uint32_t {
            attacker,
            projectile,
            target,
            lead_actor,
         };

         struct camera_shot_flag {
            enum type : uint32_t {
               position_follows_location = 0x01,
               rotation_follows_target   = 0x02,
               do_not_follow_bone        = 0x04,
               first_person_camera       = 0x08,
               no_tracer                 = 0x10,
               start_at_time_zero        = 0x20,
            };
         };
         using camera_shot_flags_t = std::underlying_type_t<camera_shot_flag::type>;

      public:
         components::model model; // MODL, MODT
         components::papyrus_attachment_data script_data; // VMAD
         //
         camera_action       action   = camera_action::shoot;
         camera_subject      location = camera_subject::attacker;
         camera_subject      target   = camera_subject::target;
         camera_shot_flags_t flags    = 0;
         struct {
            float player = 1;
            float target = 1;
            float global = 1;
         } time_multipliers;
         float minimum_time = 0;
         float maximum_time = 1;
         float target_percentage_between_actors = 0.50;
         float near_target_distance = 100;
         form_reference_t imagespace_modifier; // MNAM // value is IMAD

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