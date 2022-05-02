#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "../../helpers/grid.h"
#include "../../helpers/vector3.h"

namespace dovah::loaded_forms {
   //
   // Class for heightmapped terrain in a cell. Each landscape consists of a 33x33 grid of 
   // vertices, with data starting from the southwesternmost vertex and advancing row-by-row 
   // toward the northeasternmost vertex. The western column and southern row are meant to 
   // overlap with the vertices of the adjacent cell; if they do not, there will be tears in 
   // the landscape.
   // 
   // At run-time, landscapes are divided into four "quads" of 17x17 vertices. Each quad 
   // overlaps inward with adjacent quads for the same reason that the entire cell's worth 
   // of vertices overlaps with adajcent vertices; this means that:
   // 
   //  - The cell's center vertex exists in all four quads simultaneously.
   // 
   //  - Imagine two lines bisecting the cell along the X and Y axes. Any vertex on those 
   //    lines, except the one where they intersect, exists on two quads.
   // 
   // Within the file format, texture paint data is stored per quad, which means that when 
   // a texture is painted onto a vertex that belongs to multiple quads, that texture will 
   // in turn be written into multiple quads.
   //
   class Landscape : public Form {
      public:
         static constexpr form_type_t form_type = form_type::land;
         Landscape(const constructor_params& c) : Form(form_type, c) {};

         static constexpr int vertices_per_side  = 33;
         static constexpr int total_vertex_count = vertices_per_side * vertices_per_side;

         static constexpr int vertices_per_quad_side  = 17;
         static constexpr int total_quad_vertex_count = vertices_per_quad_side * vertices_per_quad_side;

         static constexpr int max_usable_layers_per_quad = 6;

         struct quad_indices {
            quad_indices() = delete;
            enum {
               bottom_left  = 0,
               bottom_right = 1,
               top_left     = 2,
               top_right    = 3,
            };
         };

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
         template<typename T> using grid = cobb::corner_square_grid<T, vertices_per_side>;

         // Given a vertex index within a quad, retrieve a cell-relative position.
         static void quad_offset_to_cell_coords(uint8_t quad, uint8_t index, uint8_t& x, uint8_t& y);

         // Given a quad-relative position for a vertex, retrieve a cell-relative position.
         static void quad_coords_to_cell_coords(uint8_t quad, uint8_t& x, uint8_t& y);

         static bool quad_contains_cell_coords(uint8_t quad, uint8_t x, uint8_t y);

         static void cell_coords_to_quad_coords(uint8_t quad, int8_t& x, int8_t& y); // negative == out-of-bounds

         struct vertex_color {
            uint8_t r = 255;
            uint8_t g = 255;
            uint8_t b = 255;
         };

         struct alpha_layer {
            form_reference_t texture; // ATXT
            int16_t layer = 0;        // 
            grid<float> opacities = {};
         };

         uint32_t land_flags = land_flag::all_common_flags; // DATA
         struct {
            grid<float>                heights = {}; // heights[y][x] // VHGT
            grid<cobb::vector3<float>> normals = {}; // normals[y][x] // VNML, always 0xCC3 bytes in the file. each normal is encoded as a cobb::vector3<int8_t>; convert to float by dividing by 127.0F
            grid<vertex_color>         colors  = {}; // colors[y][x]  // VCLR
         } heightmap;
         std::vector<form_reference_t> textures; // VTEX
         std::array<form_reference_t, 4> default_quad_textures; // BTXT: Base TeXTure // index == quad
         std::array<std::vector<alpha_layer>, 4> alpha_layers_by_quad; // ATXT+VTXT
         std::vector<uint8_t> mpcd; // MPCD // hkMoppCode, the pre-generated collision data for the terrain. we suspect it's optional, with the game doing collision at run-time if it's absent

         alpha_layer* get_alpha_layer(uint8_t quad, int16_t index) {
            if (quad >= 4)
               return nullptr;
            for (auto& layer : this->alpha_layers_by_quad[quad])
               if (layer.layer == index)
                  return &layer;
            return nullptr;
         }

         float minimum_height() const;
         float maximum_height() const;

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