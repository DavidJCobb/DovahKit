#pragma once
#include <cstdint>
#include "./Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./components/interior_lighting.h"
#include "./structs/color_dword.h"

namespace dovah::loaded_forms {
   class LightingTemplate : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::lighting_template;
         LightingTemplate(const constructor_params& c) : Form(form_type, c) {};

         struct axis_colors {
            color_t positive = { 255, 255, 255, 0 };
            color_t negative = { 255, 255, 255, 0 };
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct {
            color_t base_color;
            axis_colors x; // DALC+0x00
            axis_colors y; // DALC+0x08
            axis_colors z; // DALC+0x10
            color_t specular;
            float   fresnel = 1.0F;
         } ambient;
         struct {
            color_t color;
            float fade = 1;
            struct {
               int32_t xy = 0;
               int32_t z  = 0;
            } rotation;
         } directional;
         struct {
            struct {
               color_t near;
               color_t far;
            } colors;
            float clip_distance = 0;
            float far   = 0;
            float max   = 1;
            float near  = 0;
            float power = 1;
         } fog;
         struct {
            float start = 0;
            float end   = 0;
         } light_fade_distance;

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