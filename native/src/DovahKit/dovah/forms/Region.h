#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./structs/color_dword.h"

namespace dovah::loaded_forms {
   class Region : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::region;
         Region(const constructor_params& c) : Form(form_type, c) {};

         #pragma region Region Areas
            struct region_area {
               struct point {
                  float x = 0;
                  float y = 0;
               };

               uint32_t edge_falloff = 512; // RPLI
               std::vector<point> points; // RPLD[]
            };
         #pragma endregion
         #pragma region Region Data
            struct region_data_object { // RDOB (legacy) or RDOT (modern)
               form_reference_t form;                  // -> TREE|FLOR|STAT|LTEX|MSTT
               int16_t          parent_index = 0xFFFF; // negative = None // index of a previous-sibling R.D.O. to use as a parent
               float            density = 30.0F;
               uint8_t          clustering = 0;
               struct {
                  uint8_t min =  0;
                  uint8_t max = 90;
               } slope;
               uint8_t  flags = 0;
               uint16_t radius_wrt_parent = 512; // 10
               uint16_t radius = 0;              // 12
               struct {
                  float min = 0;    // 14
                  float max = 2e05; // 18
               } height;
               struct {
                  float base     = 0; // 1C
                  float variance = 0; // 20
               } sink;
               float size_variance = 0; // 24
               struct {
                  uint16_t x = 0; // 28
                  uint16_t y = 0; // 2A
                  uint16_t z = 0; // 2C
               } angle_variance;
               uint32_t unk30; // 30
            };

            struct region_data_grass_entry { // RDGS
               form_reference_t object; // -> GRAS
               form_reference_t parent; // -> LTEX
            };

            struct region_data_landscape {
               std::string texture; // ICON
            };

            struct region_data_map {
               localized_string name; // RDMP
            };

            struct region_data_sound_entry { // RDSA
               form_reference_t form; // -> SNDR|SOUN
               uint32_t         flags = 0;
               float            chance;
            };
            struct region_data_sound { // RDSA+RDMO
               std::vector<region_data_sound_entry> sounds; // RDSA
               form_reference_t music; // RDMO -> MUSC
            };

            struct region_data_weather_entry { // RDWT
               form_reference_t weather; // -> WTHR
               uint32_t         chance;
               form_reference_t global;  // -> GLOB
            };

            struct region_data {
               bool    override; // RDAT+0x04
               uint8_t priority; // RDAT+0x05
               std::variant< // indices map to the RDAT+0x00 "type" enum
                  std::monostate, // unknown/unused
                  std::monostate, // unknown/unused
                  std::vector<region_data_object>,
                  std::vector<region_data_weather_entry>,
                  region_data_map,
                  region_data_landscape,
                  std::vector<region_data_grass_entry>,
                  region_data_sound
               > data;

               constexpr auto* as_objects() { return std::get_if<2>(&this->data); }
               constexpr auto* as_weather() { return std::get_if<3>(&this->data); }
               constexpr auto* as_map() { return std::get_if<4>(&this->data); }
               constexpr auto* as_landscape() { return std::get_if<5>(&this->data); }
               constexpr auto* as_grass() { return std::get_if<6>(&this->data); }
               constexpr auto* as_sound() { return std::get_if<7>(&this->data); }
            };
         #pragma endregion

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         color_t map_color; // RCLR
         form_reference_t parent_world; // WNAM -> WRLD
         std::vector<region_area> region_areas; // (RPLI+RPLD[])[]
         std::vector<region_data> region_datas; // (RDAT+...)[]

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