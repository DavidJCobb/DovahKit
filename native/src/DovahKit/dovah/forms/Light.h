#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

namespace dovah::loaded_forms {
   class Light : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::light;
         Light(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const int32_t unlimited_duration = -1;

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               random_anim_start         = 0x00010000,
               portal_strict             = 0x00020000,
               obstacle                  = 0x02000000,
            };
         };

         struct light_flag {
            enum type : uint32_t {
               dynamic          = 0x00000001,
               can_be_carried   = 0x00000002,
               negative         = 0x00000004,
               off_by_default   = 0x00000020,
               portal_strict    = 0x00002000,
               //
               // Other bits are used internally; see the `internal_light_flag` mask. In DovahKit,
               // we map these bits to enums for friendlier access.
               //
            };
         };
         using light_flags_t = std::underlying_type_t<light_flag::type>;

      protected:
         struct internal_light_flag {
            //
            // These flags are stored in the same enumeration as `light_flag`, but in DovahKit, they are only 
            // used by the loader and are cleared from memory after we map them to "friendly" fields such as
            // `light_type` and `flicker.type`. Use those fields and consider these mask bits an implementation 
            // detail within the file format.
            //
            enum type : std::underlying_type_t<light_flag::type> {
               flicker          = 0x00000008,

               flicker_slow     = 0x00000040,
               pulse            = 0x00000080,
               pulse_slow       = 0x00000100,

               type_spot        = 0x00000200,
               type_spot_shadow = 0x00000400,
               type_hemi_shadow = 0x00000800,
               type_omni_shadow = 0x00001000,

               // "Omni" is the light type if none of the above light type bits are set.
               // "None" is the flicker mode if none of the above flicker bits are set.
            };

            static constexpr const auto all_flicker_types = flicker | flicker_slow | pulse | pulse_slow;
            static constexpr const auto all_light_types   = type_spot | type_spot_shadow | type_hemi_shadow | type_omni_shadow;
         };

      public:
         enum class emitter_type {
            omni,
            spot,        // flags & 0x0200
            spot_shadow, // flags & 0x0400
            hemi_shadow, // flags & 0x0800
            omni_shadow, // flags & 0x1000
         };
         enum class flicker_type {
            none,
            flicker,
            flicker_slow,
            pulse,
            pulse_slow,
         };

         components::papyrus_attachment_data script_data; // VMAD
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         //
         light_flags_t light_flags      = 0;
         emitter_type  light_type       = emitter_type::omni;
         int32_t       time             = unlimited_duration; // duration of temporary lights, like carried torches
         uint32_t      radius           = 16;
         float         fade             = 1.0F;   // FNAM
         color_t       color            = { 255, 255, 255, 255 };
         float         falloff_exponent = 1.0F;   // spotlights only
         float         fov              = 90.0F;  // spotlights only // degrees
         float         near_clip        = 0.001F;
         struct {
            flicker_type type   = flicker_type::none;
            float        period = 0.0F;
            struct {
               float intensity = 0.0F;
               float movement  = 0.0F;
            } amplitudes;
         } flicker;
         struct {
            localized_string name;          // FULL
            uint32_t         value  = 0;
            float            weight = 0.0F;
            form_reference_t sound;         // SNAM; form type is SNDR
            struct {
               std::string inventory; // ICON
               std::string message;   // MICO
            } icons;
         } item_data;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}