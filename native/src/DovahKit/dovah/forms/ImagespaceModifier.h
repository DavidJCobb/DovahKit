#pragma once
#include <array>
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"
#include "./structs/color_dword.h"

namespace dovah::loaded_forms {
   class ImagespaceModifier : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::imagespace_modifier;
         ImagespaceModifier(const constructor_params& c) : Form(form_type, c) {};

         static constexpr uint32_t interpolator_subrecord(uint8_t index) { return (index) | '\x00IAD'; }

         struct modifier_flag {
            enum type : uint32_t {
               animatable = 1,
            };
         };
         using modifier_flags_t = std::underlying_type_t<modifier_flag::type>;

         struct radial_blur_flag {
            enum type : uint32_t {
               use_target = 1,
            };
         };
         using radial_blur_flags_t = std::underlying_type_t<radial_blur_flag::type>;

         template<typename T>
         struct keyframe {
            float time  = 0;
            T     value = {};
         };
         //
         using interpolated_color = std::vector<keyframe<color_t>>;
         using interpolated_float = std::vector<keyframe<float>>;
         //
         struct interpolated_mult_add {
            interpolated_float mult;
            interpolated_float add;
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         modifier_flags_t flags    = 0; // DNAM+0x00
         float            duration = 0; // DNAM+0x04
         struct {
            struct {
               interpolated_float radius; // base: DNAM+0xB4; interp: BNAM
            } basic;
            struct {
               interpolated_float strength; // base: DNAM+0xF0; interp: NAM4
            } motion;
            struct {
               struct {
                  float x = 0; // DNAM+0xCC
                  float y = 0; // DNAM+0xD0
               } center;
               interpolated_float strength; // base: DNAM+0xBC; interp: RNAM
               interpolated_float ramp_up;  // base: DNAM+0xC0; interp: SNAM
               interpolated_float start;    // base: DNAM+0xC4; interp: UNAM
               radial_blur_flags_t flags = 0; // DNAM+0xC8
               struct {
                  interpolated_float start; // base: DNAM+0xE8; interp: NAM2 // start time
                  interpolated_float value; // base: DNAM+0xE4; interp: NAM1
               } ramp_down;
            } radial;
         } blurs;
         struct {
            interpolated_mult_add saturation; // base: DNAM+0x90,0x94; interp subrecord indices: 0x11, 0x51
            interpolated_mult_add brightness; // base: DNAM+0x98,0x9C; interp subrecord indices: 0x12, 0x52
            interpolated_mult_add contrast;   // base: DNAM+0xA0,0xA4; interp subrecord indices: 0x13, 0x53
            interpolated_mult_add unused;     // base: DNAM+0xA8,0xAC; interp subrecord indices: 0x14, 0x54
         } cinematic;
         struct {
            interpolated_color fade; // base: DNAM+0xEC; interp: NAM3
            interpolated_color tint; // base: DNAM+0xB0; interp: TNAM
         } colors;
         struct {
            interpolated_float strength; // base: DNAM+0xD4; interp: WNAM
            interpolated_float distance; // base: DNAM+0xD8; interp: XNAM
            interpolated_float range;    // base: DNAM+0xDC; interp: YNAM
            bool     use_target = false;   // DNAM+0xE0
            uint8_t  flags = 0;            // DNAM+0xE1
         } depth_of_field;
         struct {
            interpolated_float strength; // base: DNAM+0xB8; interp: VNAM
         } double_vision;
         struct {
            struct {
               interpolated_mult_add blur_radius;  // base: DNAM+0x10,0x14; interp subrecord indices: 0x01, 0x41
               interpolated_mult_add scale;        // base: DNAM+0x20,0x24; interp subrecord indices: 0x03, 0x43
               interpolated_mult_add threshold;    // base: DNAM+0x18,0x1C; interp subrecord indices: 0x02, 0x42
            } bloom;
            interpolated_mult_add eye_adapt_speed; // base: DNAM+0x08,0x0C; interp subrecord indices: 0x00, 0x40
            struct {
               interpolated_mult_add min;          // base: DNAM+0x28,0x2C; interp subrecord indices: 0x04, 0x44
               interpolated_mult_add max;          // base: DNAM+0x30,0x34; interp subrecord indices: 0x05, 0x45
            } target_luminescence;
            interpolated_mult_add sky_scale;       // base: DNAM+0x40,0x44; interp subrecord indices: 0x07, 0x47
            interpolated_mult_add sunlight_scale;  // base: DNAM+0x38,0x3C; interp subrecord indices: 0x06, 0x46
         } hdr; // DATA+0x08
         std::array<interpolated_mult_add, 8> unknown; // base: DNAM+0x48,0x4C ... DNAM+0x88,0x8C; interp subrecord indices are mult:[0x08, 0x10] and add:[0x48, 0x50]

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}