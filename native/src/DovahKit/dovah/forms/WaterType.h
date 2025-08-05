#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "./Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./structs/color_dword.h"
#include "helpers/vector3.h"

namespace dovah::loaded_forms {
   class WaterType : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::water_type;
         WaterType(const constructor_params& c) : Form(form_type, c) {};

         struct flag {
            enum type : uint8_t {
               causes_damage  = 1 << 0,
               enable_flowmap = 1 << 4, // SSE-only
               blend_normals  = 1 << 5, // SSE-only
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct noise_layer {
            float       amplitude_scale =   0.0F; // DNAM+0xB8,BC,C0
            std::string texture;                  // legacy: NNAM, storing all three texture paths; modern: NAM2,NAM3,NAM4
            float       uv_scale        = 100.0F; // DNAM+0xAC,B0,B4
            float       wind_direction  =   0.0F; // DNAM+0x64,68,6C
            float       wind_speed      =   0.0F; // DNAM+0x70,74,78
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name = localized_string(localized_string_type::common); // FULL
         //
         uint16_t damage_per_second = 0; // DATA // xEdit claims this is unused pre-SSE.
         struct {
            float normals     = 1.0F; // DNAM+0xD8
            float reflections = 1.0F; // DNAM+0xD0
            float refraction  = 1.0F; // DNAM+0xD4
            float specular    = 1.0F; // DNAM+0xDC
         } depth;
         struct {
            float dampen        = 10.0F;   // DNAM+0x58
            float falloff       =  0.985F; // DNAM+0x54
            float force         =  0.4F;   // DNAM+0x4C
            float starting_size =  0.01F;  // DNAM+0x48
            float velocity      =  0.6F;   // DNAM+0x50
         } displacement;
         flags_t flags = 0; // FNAM
         struct {
            struct {
               float amount = 1.0F; // DNAM+0x84
               struct {
                  float near = 0.0F; // DNAM+0x20
                  float far  = 0.0F; // DNAM+0x24
               } distance;
            } above_water;
            struct {
               float amount = 1.0F; // DNAM+0x8C
               struct {
                  float near =    0.0F; // DNAM+0x90
                  float far  = 1000.0F; // DNAM+0x94
               } distance;
            } under_water;
         } fog;
         struct {
            std::string material_id; // MNAM // vestigial, from Oblivion
            struct {
               float force    = 0.1F;   // DNAM+0x38
               float velocity = 0.6F;   // DNAM+0x3C
               float falloff  = 0.985F; // DNAM+0x40
               float dampener = 2.0F;   // DNAM+0x44
            } rain;
            struct {
               form_reference_t day;        // GNAM+0x00 -> WATR
               form_reference_t night;      // GNAM+0x04 -> WATR
               form_reference_t underwater; // GNAM+0x08 -> WATR
            } related_waters;
            uint8_t texture_blend = 50; // DNAM+0x34 // CK initializes this to 0, though?
            struct {
               float amplitude = 0.5F; // DNAM+0x08
               float frequency = 1.0F; // DNAM+0x0C
            } wave;
            struct {
               float direction = 90.0F; // DNAM+0x04
               float velocity  =  0.1F; // DNAM+0x00
            } wind;
         } legacy; // TES4 leftovers
         form_reference_t material_type; // TNAM -> MATT
         struct {
            float       falloff = 300.0F; // DNAM+0x60
            std::string flowmap_texture; // NAM5 // SSE-only
            float       flowmap_scale;   // DNAM+0xE4 // SSE-only
            std::array<noise_layer, 3> layers;
         } noise;
         uint8_t opacity = 75; // ANAM
         form_reference_t sound; // SNAM -> SNDR
         struct {
            float brightness =     1.0F; // DNAM+0xA8
            float power      =   100.0F; // DNAM+0x9C
            float radius     = 10000.0F; // DNAM+0xA4
            struct {
               float sparkle_power      =  1.0F; // DNAM+0xE0
               float sparkle_magnitude  =  1.0F; // DNAM+0xC8
               float specular_magnitude =  1.0F; // DNAM+0xCC
               float power              = 50.0F; // DNAM+0x10
            } sun;
         } specular;
         form_reference_t spell_to_apply; // XNAM -> SPEL
         form_reference_t underwater_imagespace; // INAM -> IMGS
         struct {
            uint32_t DNAM_1C =    0;
            float    DNAM_5C =    0.05F;
            float    DNAM_7C = 6200.0F; // CK initializes this to 300.0F, though?
            float    DNAM_80 =    0.2F; // CK initializes this to 300.0F, though?
            float    DNAM_88 =  900.0F;
            float    DNAM_A0 =    0.0F;
         } unknown;
         struct {
            cobb::vector3<float> linear;
            cobb::vector3<float> angular;
         } velocity;
         struct {
            struct {
               color_t deep       = {   0,   0,  25, 0 }; // DNAM+0x2C
               color_t reflection = { 255, 255, 255, 0 }; // DNAM+0x30
               color_t shallow    = {   0, 128, 128, 0 }; // DNAM+0x28
            } colors;
            float fresnel              =   0.025F; // DNAM+0x18
            float reflection_magnitude =   1.0F;   // DNAM+0xC4
            float reflectivity         =   0.5F;   // DNAM+0x14
            float refraction_magnitude = 250.0F;   // DNAM+0x98
         } water;

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