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
         static constexpr form_type_t form_type = form_type::light;
         Light(const constructor_params& c) : Form(form_type, c) {};

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
               flicker          = 0x00000008,
               off_by_default   = 0x00000020,
               flicker_slow     = 0x00000040,
               pulse            = 0x00000080,
               pulse_slow       = 0x00000100,
               type_spot        = 0x00000200, // only used by the loader, and cleared in memory; use the light type enum
               type_spot_shadow = 0x00000400, // only used by the loader, and cleared in memory; use the light type enum
               type_hemi_shadow = 0x00000800, // only used by the loader, and cleared in memory; use the light type enum
               type_omni_shadow = 0x00001000, // only used by the loader, and cleared in memory; use the light type enum
               portal_strict    = 0x00002000,
               //
               all_types = type_spot | type_spot_shadow | type_hemi_shadow | type_omni_shadow, // only used by the loader, and cleared in memory; use the light type enum
            };
         };
         using light_flags_t = std::underlying_type_t<light_flag::type>;

         enum class engine_light_type {
            omni,        // no other flags met
            spot,        // flags & 0x0200
            spot_shadow, // flags & 0x0400
            hemi_shadow, // flags & 0x0800
            omni_shadow, // flags & 0x1000
         };

         components::papyrus_attachment_data script_data; // VMAD
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::destruction_stage_data destruction_data; // DEST
         //
         light_flags_t     light_flags = 0;
         engine_light_type light_type  = engine_light_type::omni;
         int32_t  time; // duration of temporary lights, like carried torches
         uint32_t radius = 128;
         float    fade  = 1.0F; // FNAM
         color_t  color = { 255, 255, 255, 255 };
         float    falloff_exponent = 1.0F;
         float    fov       = 90.0F;
         float    near_clip = 0.0F;
         struct {
            float period = 0.01F;
            struct {
               float intensity;
               float movement;
            } amplitudes;
         } flicker;
         struct {
            localized_string name;          // FULL
            std::string      icon;          // ICON
            uint32_t         value  = 0;
            float            weight = 0.0F;
            form_reference_t sound;         // SNAM; form type is SNDR
         } item_data;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}