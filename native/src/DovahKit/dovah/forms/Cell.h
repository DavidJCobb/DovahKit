#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/interior_lighting.h"
#include "../../helpers/vector3.h"

namespace dovah::loaded_forms {
   class Cell : public Form {
      public:
         static constexpr form_type_t form_type = form_type::cell;
         Cell() : Form(form_type) {};

         struct cell_flag {
            cell_flag() = delete;
            enum {
               interior              = 0x0001,
               has_water             = 0x0002,
               cant_travel_from_here = 0x0004,
               no_lod_water          = 0x0008,
               public_area           = 0x0020,
               hand_changed          = 0x0040,
               show_sky              = 0x0080,
               use_sky_lighting      = 0x0100,
            };
         };
         struct land_flag {
            land_flag() = delete;
            enum {
               force_hide_quad_1 = 0x01,
               force_hide_quad_2 = 0x02,
               force_hide_quad_3 = 0x04,
               force_hide_quad_4 = 0x08,
            };
         };

         /*// whoops, this one is for WRLD
         struct max_height_data_t {
            struct quad_heights {
               int8_t sw;
               int8_t se;
               int8_t nw;
               int8_t ne;
            };
            //
            struct {
               int16_t x;
               int16_t y;
            } min;
            struct {
               int16_t x;
               int16_t y;
            } max;
            std::vector<quad_heights> cells;
         };
         //*/
         struct max_height_data_t {
            float offset;
            std::array<std::array<int8_t, 32>, 32> grid;
            bool  present = false;
         };

         localized_string name; // FULL
         uint16_t cell_flags = 0; // DATA
         struct {
            int32_t x = 0;
            int32_t y = 0;
         } grid_coords; // XCLC
         uint32_t  land_flags = 0;    // XCLC
         form_id_t encounter_zone_ID; // XEZN
         form_id_t imagespace_ID;     // IMGS
         form_id_t location_ID;       // XLCN
         form_id_t music_type_ID;     // XCMO
         form_id_t sky_region_ID;     // XCCM
         struct {
            components::interior_lighting lighting; // XCLL
            form_id_t acoustic_space_ID;     // XCAS
            form_id_t lighting_template_ID;  // LTMP
            form_id_t lock_list_ID;          // XILL (FLST, NPC_)
            form_id_t owner_ID;              // XOWN (FACT, NPC_)
         } interior;
         struct {
            std::vector<uint8_t>   occlusion_data;        // TVDT
            max_height_data_t      max_height_data;       // MHDT
            uint32_t               deprecated_land_flags; // LNAM // leftover flags, moved to XCLC
            std::vector<form_id_t> containing_region_IDs; // XCLR
         } exterior;
         struct {
            float       height;          // XCLW
            form_id_t   type;            // XCWT
            std::string environment_map; // XWEM
            std::string noise_texture;   // XNAM
            struct {
               bool present = false;
               cobb::vector3<float> linear;
               uint32_t unk0C;
               cobb::vector3<float> angular;
            } velocity; // XWCU
            // TODO: XWCN
            // TODO: XWCS
         } water;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
         //
      protected:
         virtual bool _save_impl(tes_file_writing::record& record) override;
   };
}