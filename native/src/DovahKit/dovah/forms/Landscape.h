#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "../../helpers/vector3.h"

namespace dovah::loaded_forms {
   class Landscape : public Form {
      #include "impl/form_subclass_components.txt"
      public:
         static constexpr form_type_t form_type = form_type::land;
         Landscape(const constructor_params& c) : Form(form_type, c) {};

         static constexpr int vertices_per_side  = 33;
         static constexpr int total_vertex_count = vertices_per_side * vertices_per_side;

         struct land_flag {
            land_flag() = delete;
            enum : uint32_t {
               has_heightmap = 0x001, // if this flag is not set, the game skips VHGT and VNML even if they're present
               has_colors    = 0x002, // if this flag is not set, the game skips VCLR even if it's present
               has_layers    = 0x004,
               unknown_4     = 0x008, // this flag is forcibly cleared when the game loads flags from 'DATA'
               unknown_5     = 0x010,
               //
               has_mopp_code   = 0x400, // the game seems to ignore this when loading 'DATA'?
               successful_mopp = 0x800, // run-time flag set if LAND/MPCD is loaded successfully; shouldn't need to be set in a file; probably shouldn't ever be set in a file
               //
               all_common_flags = has_heightmap | has_colors | has_layers,
            };
         };

         // Bethesda splits the landscape up into four quads. We're not gonna bother.
         // Data starts from the southwesternmost vertex and advances toward the 
         // northeasternmost vertex, so positive Y is north and negative Y is south. 
         // The western column and southern row must overlap with those of the adjoining 
         // cells, or there will be tears in the landscape.
         template<typename T> struct grid {
            std::array<T, vertices_per_side * vertices_per_side> list = {};

            T& at(int x, int y) noexcept { return list[y * vertices_per_side + x]; }
            const T& at(int x, int y) const noexcept { return list[y * vertices_per_side + x]; }
         };

         struct vertex_color {
            uint8_t r = 255;
            uint8_t g = 255;
            uint8_t b = 255;
         };

         struct alpha_entry {
            uint16_t index = 0; // vertex index
            float    value = 0; // opacity
         };
         struct alpha_layer {
            form_reference_t texture; // ATXT
            uint8_t quad  = 0;        // 
            int16_t layer = 0;        // 
            std::vector<alpha_entry> alpha; // VTXT
         };

         uint32_t land_flags = land_flag::all_common_flags; // DATA
         struct {
            grid<float>                heights; // heights[y][x] // VHGT
            grid<cobb::vector3<float>> normals; // normals[y][x] // VNML, always 0xCC3 bytes in the file. each normal is encoded as a cobb::vector3<int8_t>; convert to float by dividing by 127.0F
            grid<vertex_color>         colors;  // colors[y][x]  // VCLR
         } heightmap;
         std::vector<form_reference_t> textures; // VTEX
         std::array<form_reference_t, 4> default_quad_textures; // BTXT: Base TeXTure // index == quad
         std::vector<alpha_layer> alpha_layers;
         std::vector<uint8_t> mpcd; // MPCD // hkMoppCode, the pre-generated collision data for the terrain. we suspect it's optional, with the game doing collision at run-time if it's absent

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}