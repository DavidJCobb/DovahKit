#pragma once
#include "./Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./structs/color_dword.h"
#include "helpers/vector3.h"

namespace dovah::loaded_forms {
   class EffectShader : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::effect_shader;
         EffectShader(const constructor_params& c) : Form(form_type, c) {};

         enum class blend_mode : uint32_t {
            zero = 1,
            one,
            src_color,
            src_inv_color,
            src_alpha,
            src_inv_alpha,
            dst_color,
            dst_inv_color,
            dst_alpha,
            dst_inv_alpha,
            src_alpha_sat,
         };
         enum class blend_operation : uint32_t {
            add = 1,
            sub,
            sub_reverse,
            min,
            max,
         };
         enum class z_test_function : uint32_t {
            equal = 3,
            normal,
            greater,
            greater_equal = 7,
            always = 8,
         };

         struct flag {
            enum type : uint32_t {
               no_membrane = 1 << 0,
               membrane_greyscale_color = 1 << 1,
               membrane_greyscale_alpha = 1 << 2,
               no_particle = 1 << 3,
               invert_edge_effect = 1 << 4,
               skin_only = 1 << 5,
               ignore_alpha = 1 << 6,
               projected_uv = 1 << 7,
               ignore_base_geom_alpha = 1 << 8,
               lighting = 1 << 9,
               no_weapons = 1 << 10,
               particle_animated = 1 << 15,
               particle_greyscale_color = 1 << 16,
               particle_greyscale_alpha = 1 << 17,
               use_blood_geom = 1 << 24,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         struct legacy_flag {
            enum type : uint8_t {
               no_membrane = 1 << 0,
               membrane_greyscale_color = 1 << 1,
               membrane_greyscale_alpha = 1 << 2,
               no_particle = 1 << 3,
               invert_edge_effect = 1 << 4,
               skin_only = 1 << 5,
               ignore_alpha = 1 << 6,
               projected_uv = 1 << 7,
            };
         };
         using legacy_flags_t = std::underlying_type_t<legacy_flag::type>;

         struct alpha_parameters {
            struct {
               float frequency = 1;
               float amplitude = 0;
            } pulse;
            struct {
               float full       = 1;
               float persistent = 0;
            } ratios;
            struct {
               float fade_in  = 0;
               float fade_out = 0;
               float full     = 0;
            } times;
         };
         struct blend_parameters {
            blend_mode      src    = blend_mode::src_alpha;
            blend_mode      dst    = blend_mode::src_inv_alpha;
            blend_operation op     = blend_operation::add;
            z_test_function z_test = z_test_function::equal;
         };
         struct color_key {
            color_t color;
            float   alpha = 1;
            float   time  = 0;
         };
         struct texture_scale {
            float u = 1;
            float v = 1;
         };

         struct float_with_variance {
            float base     = 0;
            float variance = 0;
         };
         struct uint_with_variance {
            uint32_t base = 0;
            uint32_t variance = 0;
         };

         struct particle_scale_key {
            float scale = 1;
            float time  = 0;
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD

         form_reference_t ambient_sound;
         flags_t flags = 0;
         legacy_flags_t legacy_flags = 0; // DATA+0x00
         struct {
            blend_parameters blend;
            struct {
               alpha_parameters alpha;
               color_t          color;
               float            falloff = 1;
            } edge;
            struct {
               alpha_parameters alpha;
               std::array<color_key, 3> color_keys;
               struct {
                  std::string main;    // ICON
                  std::string palette; // NAM8
                  std::string holes;   // NAM7
                  texture_scale scale;
                  texture_scale speed;
               } textures;
            } fill;
            struct {
               float start_time  = 0;
               float end_time    = 10;
               float start_value = 255;
               float end_value   = 0;
            } holes;
         } membrane;
         struct {
            blend_parameters         blend;
            std::array<color_key, 3> color_keys = {
               color_key{ .color = {.hex = 0xFFFFFF}, .time = 0.0 },
               color_key{ .color = {.hex = 0xFFFFFF}, .time = 0.5 },
               color_key{ .color = {.hex = 0xFFFFFF}, .time = 1.0 }
            };
            struct {
               float_with_variance lifetime = { 1, 0 };
               struct {
                  struct {
                     cobb::vector3<float> absolute;
                     float                along_normal = 0;
                  } acceleration;
                  float explosion_wind_speed = 0;
                  float_with_variance  initial_position; // along normal
                  float_with_variance  initial_rotation; // about normal
                  float_with_variance  initial_speed;    // along normal
                  float_with_variance  rotation_speed;   // about normal
                  cobb::vector3<float> initial_velocity; // absolute
               } movement;
               struct {
                  struct {
                     float full       = 1;
                     float persistent = 1;
                  } counts;
                  uint16_t scene_graph_emit_depth_limit = 0;
                  struct {
                     float ramp_up   = 0;
                     float ramp_down = 0;
                     float full      = 0;
                  } times;
               } spawn;
            } behavior;
            struct {
               form_reference_t form;
               struct {
                  float start = 1;
                  float end   = 1;
               } scales;
               struct {
                  float fade_in   = 1;
                  float fade_out  = 1;
                  float scale_in  = 1;
                  float scale_out = 1;
               } times;
            } debris;
            std::array<particle_scale_key, 2> scale_keys = { particle_scale_key{1,0}, particle_scale_key{1,1} };
            struct {
               std::string main;    // ICO2
               std::string palette; // NAM9
               struct {
                  uint_with_variance start_frame;
                  uint_with_variance loop_start_frame;
                  uint32_t           end_frame = 0;
                  uint_with_variance frame_count;
               } animation;
               struct {
                  uint32_t u = 1;
                  uint32_t v = 1;
               } count;
            } textures;
         } particle;
         float    data_unk108 = 0; // DATA+0x108
         color_t  data_unk10C;     // DATA+0x10C
         float    data_unk158 = 1; // DATA+0x158

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}