#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "./Form.h"
#include "./_common.h"
#include "./components/model.h"
#include "./components/papyrus.h"
#include "./structs/color_dword.h"
#include "./structs/directional_ambient_lighting_colors.h"

namespace dovah::loaded_forms {
   class Weather : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::weather;
         Weather(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_cloud_layer_count = 32;

         struct cloud_layer_time_params {
            float   alpha = 1; // JNAM
            color_t color;     // PNAM
         };
         struct cloud_layer {
            std::string texture; // ('TX00'+(uint8_t)index)
            struct {
               cloud_layer_time_params sunrise;
               cloud_layer_time_params day;
               cloud_layer_time_params sunset;
               cloud_layer_time_params night;
            } time_of_day;
            struct {
               uint8_t x = 0; // QNAM
               uint8_t y = 0; // RNAM (or ONAM for legacy)
            } speed;
         };

         struct volumetric_lighting { // HNAM (SSE-only)
            form_reference_t sunrise; // HNAM+0x00 -> VOLI
            form_reference_t day;     // HNAM+0x04 -> VOLI
            form_reference_t sunset;  // HNAM+0x08 -> VOLI
            form_reference_t night;   // HNAM+0x0C -> VOLI
         };

         struct weather_colors_by_time {
            color_t sunrise;
            color_t day;
            color_t sunset;
            color_t night;
         };

         enum class weather_sound_type : uint32_t {
            default_,
            precipitation,
            wind,
            thunder,
         };
         struct weather_sound { // SNAM
            form_reference_t   form; // SNAM+0x00 -> SNDR/SOUN
            weather_sound_type type = weather_sound_type::default_; // SNAM+0x04
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         uint32_t max_cloud_layers = 29; // LNAM
         components::model aurora; // MODL+MODT
         struct {
            uint32_t disabled_layers = 0xFFFFFFFF; // NAM1 // bitset
            std::array<cloud_layer, max_cloud_layer_count> layers;
         } clouds;
         union _ {
            ~_() { list.~array(); }
            //
            std::array<weather_colors_by_time, 17> list = {};
            struct {
               weather_colors_by_time sky_upper;
               weather_colors_by_time fog_near;
               weather_colors_by_time unused;
               weather_colors_by_time ambient;
               weather_colors_by_time sunlight;
               weather_colors_by_time sun;
               weather_colors_by_time stars;
               weather_colors_by_time sky_lower;
               weather_colors_by_time horizon;
               weather_colors_by_time effect_lighting;
               weather_colors_by_time cloud_lod_diffuse;
               weather_colors_by_time cloud_lod_ambient;
               weather_colors_by_time fog_far;
               weather_colors_by_time sky_statics;
               weather_colors_by_time water_multiplier;
               weather_colors_by_time sun_glare;  // legacy: NAM3
               weather_colors_by_time moon_glare; // legacy: NAM2
            };
         } colors; // NAM0
         union {
            std::array<structs::directional_ambient_lighting_colors, 4> list = {};
            struct {
               structs::directional_ambient_lighting_colors sunrise; // DALC[0]
               structs::directional_ambient_lighting_colors day;     // DALC[1]
               structs::directional_ambient_lighting_colors sunset;  // DALC[2]
               structs::directional_ambient_lighting_colors night;   // DALC[3]
            };
         } directional_ambient_lighting;
         struct {
            struct {
               float near;  // FNAM+0x00
               float far;   // FNAM+0x04
               float power; // FNAM+0x10
               float max;   // FNAM+0x18
            } day;
            struct {
               float near;  // FNAM+0x08
               float far;   // FNAM+0x0C
               float power; // FNAM+0x14
               float max;   // FNAM+0x1C
            } night;
         } fog_distance; // FNAM
         struct {
            form_reference_t sunrise; // IMSP+0x00 -> IMGS
            form_reference_t day;     // IMSP+0x04 -> IMGS
            form_reference_t sunset;  // IMSP+0x08 -> IMGS
            form_reference_t night;   // IMSP+0x0C -> IMGS
         } imagespaces; // IMSP
         std::vector<form_reference_t> sky_statics; // TNAM[] -> STAT
         std::vector<weather_sound>    sounds;      // SNAM[]
         std::optional<volumetric_lighting> volumetric; // HNAM (SSE-only)
         //
         uint8_t flags = 0; // DATA+0x0B
         struct {
            form_reference_t form; // MNAM -> SPGD
            uint8_t begin_fade_in; // DATA+0x06
            uint8_t end_fade_out;  // DATA+0x07
         } precipitation;
         struct {
            form_reference_t lens_flare; // GNAM -> LENS (SSE-only)
            uint8_t glare; // DATA+0x04
            uint8_t damage; // DATA+0x05
         } sun;
         struct {
            uint8_t begin_fade_in; // DATA+0x08
            uint8_t end_fade_out; // DATA+0x09
            uint8_t frequency; // DATA+0x0A
            struct {
               uint8_t r; // DATA+0x0C
               uint8_t g; // DATA+0x0D
               uint8_t b; // DATA+0x0E
            } lightning_color;
         } thunderstorm;
         struct {
            form_reference_t form; // NNAM -> RFCT
            uint8_t begin_fade_in; // DATA+0x0F // [0, 1]
            uint8_t end_fade_out;  // DATA+0x10 // [0, 1]
         } visual_effect;
         struct {
            struct {
               uint8_t base;     // DATA+0x11 // [0, 360]
               uint8_t variance; // DATA+0x12 // [0, 100]
            } direction;
            uint8_t speed; // DATA+0x00
         } wind;
         uint8_t trans_delta; // DATA+0x03

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