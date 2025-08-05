#pragma once
#include <cstdint>
#include <optional>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"

namespace dovah::loaded_forms {
   class Projectile : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::projectile;
         Projectile(const constructor_params& c) : Form(form_type, c) {};

         struct flag {
            enum type : uint16_t {
               hitscan         = 1 << 0,
               explosion       = 1 << 1,
               alt_trigger     = 1 << 2,
               muzzle_flash    = 1 << 3,
               //
               can_be_disabled = 1 << 5,
               can_be_taken    = 1 << 6,
               supersonic      = 1 << 7,
               pins_limbs      = 1 << 8,
               pass_through_small_transparent = 1 <<  9,
               disable_combat_aim_correction  = 1 << 10,
               rotation = 1 << 11,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         enum class projectile_type : uint16_t {
            missile = 0x01,
            lobber  = 0x02,
            beam    = 0x04,
            flame   = 0x08,
            cone    = 0x10,
            barrier = 0x20,
            arrow   = 0x40,
         };

      public:
         components::object_bounds bounds; // OBND
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::model model; // MODL, MODT
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name; // FULL
         //
         flags_t         flags = 0;
         projectile_type type  = projectile_type::arrow;
         //
         float   cone_spread       = 0;
         float   collision_radius  = 10;
         float   fade_duration     = 0.5;
         float   gravity           = 1000;
         float   lifetime          = 0;
         float   impact_force      = 0;
         float   range             = 0;
         float   relaunch_interval = 0;
         float   speed             = 10000;
         float   tracer_chance     = 0;
         form_reference_t collision_layer;
         form_reference_t decal;
         form_reference_t default_weapon_source;
         struct {
            struct {
               float proximity = 0;
               float timer     = 0;
            } alt_trigger;
            form_reference_t form;
         } explosion;
         form_reference_t   light;
         detection_loudness loudness = detection_loudness::normal; // VNAM
         struct {
            float             duration;
            form_reference_t  light;
            components::model model; // NAM1 for filename; NAM2 for info
         } muzzle_flash;
         struct {
            form_reference_t countdown;
            form_reference_t disarm;
            form_reference_t flyby;
         } sounds;

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void setup(const file_load_order&) noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}