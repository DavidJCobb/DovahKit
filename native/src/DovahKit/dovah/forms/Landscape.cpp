#include "Landscape.h"
//
#include "helpers/simd/min_and_max.h"
#include <intrin.h>
#include "helpers/cpuinfo.h"
//
#include "_common_cpp.h"
#include "../notice_code_list.h"

#include "Cell.h"

#include "../notices/form_load_warnings/by_form_type/landscape/excess_layers_per_quad.h"
#include "../notices/form_load_warnings/by_form_type/landscape/invalid_quad_for_land_texture.h"
#include "../notices/form_save_errors/by_form_type/landscape/heightmap_contains_too_steep_a_slope.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::landscape;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::landscape;
   }

   static constexpr float vertex_distance = dovah::loaded_forms::Cell::side_length / (dovah::loaded_forms::Landscape::vertices_per_side - 1);
}

namespace dovah::loaded_forms {
   float Landscape::minimum_height() const {
      auto& list = this->heightmap.heights.list();
      if (!std::is_constant_evaluated()) {
         if (cobb::cpuinfo::get().extension_support.sse_1) {
            return cobb::simd::minimum_of_list(list);
         }
      }
      float min = std::numeric_limits<float>::max();
      for (auto f : list)
         if (f < min)
            min = f;
      return min;
   }
   float Landscape::maximum_height() const {
      auto& list = this->heightmap.heights.list();
      if (!std::is_constant_evaluated()) {
         if (cobb::cpuinfo::get().extension_support.sse_1) {
            return cobb::simd::maximum_of_list(list);
         }
      }
      float max = std::numeric_limits<float>::lowest(); // ::min() isn't actually the minimum for floating-point types
      for (auto f : list)
         if (f > max)
            max = f;
      return max;
   }

   void Landscape::recalc_normals() {
      this->recalc_normals_to(this->heightmap.normals.list());
   }
   void Landscape::recalc_normals_to(std::array<cobb::vector3<float>, total_vertex_count>& out) const {
      using vector3 = cobb::vector3<float>;
      //
      auto _height_at = [this](int x, int y) -> vector3 {
         return { x * vertex_distance, y * vertex_distance, this->heightmap.heights.item(x, y) };
      };

      for (auto& item : out)
         item = { 0, 0, 0 };
      for (int y = 0; y < vertices_per_side - 1; ++y) {
         for (int x = 0; x < vertices_per_side - 1; ++x) {
            vector3 a = _height_at(x,     y);
            vector3 b = _height_at(x + 1, y);
            vector3 c = _height_at(x,     y + 1);
            vector3 d = _height_at(x + 1, y + 1);
            //
            auto normal = (b - a).cross(c - a);
            out[(y + 0) * vertices_per_side + (x + 0)] += normal;
            out[(y + 1) * vertices_per_side + (x + 0)] += normal;
            out[(y + 0) * vertices_per_side + (x + 1)] += normal;
            out[(y + 1) * vertices_per_side + (x + 1)] += normal;
         }
      }
      for (auto& item : out)
         item.normalize();
   }

   void Landscape::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      this->land_flags = 0;
      //
      form_reference_t form_id;
      int8_t  last_alpha_quad  = -1;
      int32_t last_alpha_layer = -1;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->land_flags);
               break;
            case 'VCLR': // vertex colors
               // The game skips this if the "has color" flag is not set, but that behavior wouldn't be useful for 
               // editor programs, so we won't replicate it.
               for (int i = 0; i < total_vertex_count; ++i) {
                  if (!subrecord.is_in_bounds(3))
                     break;
                  uint8_t r;
                  uint8_t g;
                  uint8_t b;
                  subrecord.unchecked_read(r);
                  subrecord.unchecked_read(g);
                  subrecord.unchecked_read(b);
                  //
                  this->heightmap.colors.by_flat_index(i) = { r, g, b };
               }
               break;
            case 'VHGT': // vertex heights
               // The game skips this if the "has heightmap" flag is not set, but that behavior wouldn't be useful 
               // for editor programs, so we won't replicate it.
               if (!subrecord.is_in_bounds(sizeof(float) + total_vertex_count))
                  break;
               {
                  //
                  // Landscapes are encoded as follows:
                  // 
                  //  - There is a base height consisting of a float divided by eight, followed by a grid of 33x33 
                  //    vertex deltas encoded as signed bytes.
                  // 
                  //  - The first vertex in each row is encoded as the delta from the first vertex in the previous 
                  //    row, divided by eight. The first vertex in the first row has a delta of zero.
                  // 
                  //  - After that, each vertex in a row is encoded as the delta from the previous vertex, divided 
                  //    by eight.
                  // 
                  // Accordingly, the following relationships exist, in order of decreasing priority:
                  // 
                  //  - bytes[0][0] == 0
                  // 
                  //  - bytes[0][y] == (height[0][y] - height[0][y - 1]) / 8
                  // 
                  //  - bytes[x][y] == (height[x][y] - height[x - 1][y]) / 8
                  //
                  float base_offset = 0.0F;
                  float span_offset = 0.0F;
                  subrecord.unchecked_read(base_offset);
                  base_offset *= 8.0F;
                  //
                  for (int i = 0; i < total_vertex_count; ++i) {
                     int8_t value;
                     subrecord.unchecked_read(value);
                     //
                     int x = i % vertices_per_side; // col
                     int y = i / vertices_per_side; // row
                     if (x == 0) {
                        //
                        // The first value in a row serves as a basis for the entire row.
                        //
                        span_offset = 0.0F;
                        base_offset += (float)value * 8.0F;
                     } else {
                        span_offset += (float)value * 8.0F;
                     }
                     this->heightmap.heights.by_flat_index(i) = base_offset + span_offset;
                  }
               }
               subrecord.skip_bytes(3); // padding
               break;
            case 'VNML': // vertex normals
               // The game skips this if the "has heightmap" flag is not set, but that behavior wouldn't be useful 
               // for editor programs, so we won't replicate it.
               for (int i = 0; i < total_vertex_count; ++i) {
                  if (!subrecord.is_in_bounds(3))
                     break;
                  int8_t x;
                  int8_t y;
                  int8_t z;
                  subrecord.unchecked_read(x);
                  subrecord.unchecked_read(y);
                  subrecord.unchecked_read(z);
                  //
                  auto& vec = this->heightmap.normals.by_flat_index(i);
                  vec.x = (float)x / 127.0F;
                  vec.y = (float)y / 127.0F;
                  vec.z = (float)z / 127.0F;
                  if (vec.length() < 0.000001F) {
                     vec.x = vec.y = vec.z = 0.0F;
                  } else {
                     vec.normalize();
                  }
               }
               break;
            case 'BTXT':
               if (subrecord.is_in_bounds(4)) {
                  form_reference_t ref;
                  subrecord.read(ref);
                  intfc.warn_if_ref_is_wrong_type(ref, form_type::land_texture, subrecord.signature());
                  //
                  uint8_t quad;
                  if (subrecord.read(quad)) {
                     if (quad < 4)
                        this->default_quad_textures[quad] = ref;
                     else {
                        specific_load_warnings::invalid_quad_for_land_texture notice(
                           this->stub,
                           specific_load_warnings::invalid_quad_for_land_texture::texture_type::default_texture,
                           quad
                        );
                        intfc.log_load_warning(notice);
                     }
                  }
                  subrecord.skip_bytes(3); // BTXT and ATXT have the same header, but BTXT doesn't use the layer index. advise writing a -1 layer index when saving
               }
               break;
            case 'ATXT':
               {
                  uint8_t quad;
                  //
                  alpha_layer layer;
                  subrecord.read(layer.texture);
                  subrecord.read(quad);
                  subrecord.skip_bytes(1);
                  subrecord.read(layer.layer);
                  if (quad > 3) {
                     specific_load_warnings::invalid_quad_for_land_texture notice(
                        this->stub,
                        specific_load_warnings::invalid_quad_for_land_texture::texture_type::blended_texture,
                        quad,
                        layer.layer
                     );
                     intfc.log_load_warning(notice);
                     //
                     // Discard anything that would go into a bad quad. Excess layers are sensible to keep 
                     // around within an editor, because those can arise from a user clumsily painting a 
                     // landscape, and the user may want to edit them in a sensible way; however, excess 
                     // quads can only be garbage data.
                     //
                     last_alpha_quad  = -1;
                     last_alpha_layer = -1;
                     break;
                  }
                  if (layer.layer >= max_usable_layers_per_quad) {
                     specific_load_warnings::excess_layers_per_quad notice(
                        this->stub,
                        quad,
                        layer.layer,
                        subrecord.signature(),
                        layer.texture.get_form_stub()
                     );
                     intfc.log_load_warning(notice);
                  }
                  //
                  bool existing = false;
                  for (auto& prior : this->alpha_layers_by_quad[quad]) {
                     if (prior.layer == layer.layer) {
                        prior.texture = layer.texture;
                        existing = true;
                        break;
                     }
                  }
                  if (!existing) {
                     this->alpha_layers_by_quad[quad].push_back(layer);
                  }
                  last_alpha_quad  = quad;
                  last_alpha_layer = layer.layer;
               }
               break;
            case 'VTXT':
               if (subrecord.size() & 7) { // the game skips VTXT subrecords with an "uneven" length
                  break;
               }
               if (last_alpha_quad < 0 || last_alpha_layer < 0) { // the game loads only the first VTXT it sees for an ATXT
                  break;
               }
               if (subrecord.is_in_bounds(8)) {
                  auto* ptr   = this->get_alpha_layer(last_alpha_quad, last_alpha_layer);
                  assert(ptr);
                  auto& layer = *ptr;
                  if (layer.layer < 0)
                     break;
                  while (subrecord.is_in_bounds(8)) {
                     uint16_t vertex;
                     float    opacity;
                     subrecord.unchecked_read(vertex);
                     subrecord.skip_bytes(2);
                     subrecord.unchecked_read(opacity);
                     //
                     if (vertex > vertices_per_quad_side * vertices_per_quad_side)
                        continue;
                     uint8_t x = vertex % vertices_per_quad_side;
                     uint8_t y = vertex / vertices_per_quad_side;
                     quad_coords_to_cell_coords(last_alpha_quad, x, y);
                     //
                     if (opacity < 0.0F)
                        opacity = 0.0F;
                     else if (opacity > 1.0F)
                        opacity /= 100.0F;
                     //
                     // The game discards vertex blend data if it fails to meet the following constraints:
                     // 
                     //  - Vertex index out of bounds
                     //  - Quad   index out of bounds
                     //  - Layer  index out of bounds
                     // 
                     // Of these three, we already validated the quad when reading an alpha layer, and we want 
                     // to load out-of-bounds layers since they may be relevant to editing. We've validated the 
                     // vertex index above as well.
                     //
                     layer.opacities.item(x, y) = opacity;
                  }
                  //
                  last_alpha_quad  = -1;
                  last_alpha_layer = -1;
               }
               break;
            case 'VTEX':
               while (subrecord.is_in_bounds(4)) {
                  auto& ref = this->textures.emplace_back();
                  subrecord.unchecked_read(ref);
                  intfc.warn_if_ref_is_wrong_type(ref, form_type::land_texture, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Landscape::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      std::array<form_id_t, 4> base_textures;
      std::vector<form_id_t>   layer_textures;
      std::vector<form_id_t>   general_textures;
      form_id_t id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
            case 'VCLR':
            case 'VHGT':
            case 'VNML':
               break;
            case 'BTXT':
               {
                  subrecord.read(id);
                  uint8_t quad;
                  subrecord.read(quad);
                  subrecord.skip_bytes(3);
                  //
                  if (quad > 3) { // our loader doesn't store bad quads
                     break;
                  }
                  base_textures[quad] = id;
               }
               break;
            case 'ATXT':
               {
                  subrecord.read(id);
                  uint8_t quad;
                  subrecord.read(quad);
                  subrecord.skip_bytes(3);
                  //
                  if (quad > 3) { // our loader doesn't store bad quads
                     break;
                  }
                  layer_textures.push_back(id);
               }
               break;
            case 'VTXT':
               break;
            case 'VTEX':
               while (subrecord.is_in_bounds(4)) {
                  subrecord.unchecked_read(id);
                  general_textures.push_back(id);
               }
               break;
         }
      }
      for (auto id : base_textures)
         uib.add_outbound_reference(id);
      for (auto id : layer_textures)
         uib.add_outbound_reference(id);
      for (auto id : general_textures)
         uib.add_outbound_reference(id);
   }
   bool Landscape::_clone_impl(Form* out) const noexcept {
      if (out->type != form_type)
         return false;
      auto* copy = (Landscape*)out;
      //
      copy->land_flags = this->land_flags;
      copy->heightmap.heights = this->heightmap.heights;
      copy->heightmap.normals = this->heightmap.normals;
      copy->heightmap.colors  = this->heightmap.colors;
      copy_form_reference_list(*copy, copy->textures, this->textures);
      for (size_t i = 0; i < this->default_quad_textures.size(); ++i)
         copy->default_quad_textures[i].set(*copy, this->default_quad_textures[i]);
      //
      for (size_t i = 0; i < this->alpha_layers_by_quad.size(); ++i) {
         auto& src_list = this->alpha_layers_by_quad[i];
         auto& dst_list = copy->alpha_layers_by_quad[i];
         assert(dst_list.empty());
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t j = 0; j < size; ++j) {
            auto& src = src_list[j];
            auto& dst = dst_list[j];
            dst.texture.set(*copy, src.texture);
            dst.layer     = src.layer;
            dst.opacities = src.opacities;
         }
      }
      //
      copy->mpcd = this->mpcd;
      //
      return true;
   }
   bool Landscape::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->land_flags);
      DATA.close();
      //
      auto& VNML = record.open_next_subrecord('VNML');
      for (int i = 0; i < total_vertex_count; ++i) {
         auto& vec = this->heightmap.normals.by_flat_index(i);
         //
         int8_t x = 0;
         int8_t y = 0;
         int8_t z = 0;
         if (vec.length() >= 0.000001F) {
            x = vec.x * 127.0F;
            y = vec.y * 127.0F;
            z = vec.z * 127.0F;
         }
         VNML.write(x);
         VNML.write(y);
         VNML.write(z);
      }
      VNML.close();
      //
      auto& VHGT = record.open_next_subrecord('VHGT');
      {
         auto& list = this->heightmap.heights.list();
         //
         // Landscapes are encoded as follows:
         // 
         //  - There is a base height consisting of a float divided by eight, followed by a grid of 33x33 
         //    vertex deltas encoded as signed bytes.
         // 
         //  - The first vertex in each row is encoded as the delta from the first vertex in the previous 
         //    row, divided by eight. The first vertex in the first row has a delta of zero.
         // 
         //  - After that, Each vertex in a row is encoded as the delta from the previous vertex, divided 
         //    by eight.
         // 
         // Accordingly, the following relationships exist, in order of decreasing priority:
         // 
         //  - bytes[0][0] == 0
         // 
         //  - bytes[0][y] == (height[0][y] - height[0][y - 1]) / 8
         // 
         //  - bytes[x][y] == (height[x][y] - height[x - 1][y]) / 8
         //
         float base_offset = floor(list[0] / 8.0F);
         float span_offset = 0.0F;
         VHGT.write(base_offset);
         for (int i = 0; i < total_vertex_count; ++i) {
            int x = i % vertices_per_side; // col
            int y = i / vertices_per_side; // row
            //
            int8_t out = 0;
            float  raw = 0.0F;
            if (x == 0) {
               if (y != 0) {
                  int j = i - vertices_per_side; // list[j] == height[0][y - 1]
                  raw = round((list[i] - list[j]) / 8.0F);
               }
            } else {
               raw = round((list[i] - list[i - 1]) / 8.0F);
            }
            if (raw < std::numeric_limits<int8_t>::min() || raw > std::numeric_limits<int8_t>::max()) {
               detailed_notice error;
               error.code = notice_code::landscape_heights_are_too_steep;
               error.set_cause_form(this->stub);
               error.set_cause_subrecord('VHGT');
               error.extra_integers[0] = i;
               intfc.set_save_error(error);
               //
               return false;
            }
            out = raw;
            VHGT.write(out);
         }
         VHGT.skip_bytes(3);
      }
      VHGT.close();
      //
      auto& VCLR = record.open_next_subrecord('VCLR');
      for (int i = 0; i < total_vertex_count; ++i) {
         auto& color = this->heightmap.colors.by_flat_index(i);
         VCLR.write(color.r);
         VCLR.write(color.g);
         VCLR.write(color.b);
      }
      VCLR.close();
      //
      // Official files have blend data sorted by quad; BTXT, ATXT, and VTXT, per quad.
      //
      for (size_t quad = 0; quad < 4; ++quad) {
         auto& bases = this->default_quad_textures;
         {  // BTXT
            auto& ref = bases[quad];
            if (ref == nullptr)
               continue;
            auto& BTXT = record.open_next_subrecord('BTXT');
            BTXT.write(ref);
            BTXT.write(uint8_t(quad));
            BTXT.skip_bytes(1);
            BTXT.write(int16_t(-1));
            BTXT.close();
         }
         //
         // Blends:
         //
         auto& list = this->alpha_layers_by_quad[quad];
         for (auto& layer : list) {
            std::vector<uint16_t> indices;
            for (uint16_t i = 0; i < decltype(layer.opacities)::area; ++i) {
               const auto f = layer.opacities.by_flat_index(i);
               if (f > 0.0F)
                  indices.push_back(i);
            }
            if (indices.empty())
               continue;
            //
            auto& ATXT = record.open_next_subrecord('ATXT');
            ATXT.write(layer.texture);
            ATXT.write(uint8_t(quad));
            ATXT.skip_bytes(1);
            ATXT.write(layer.layer);
            ATXT.close();
            //
            auto& VTXT = record.open_next_subrecord('VTXT');
            for (const auto i : indices) {
               VTXT.write(uint16_t(i));
               VTXT.skip_bytes(2);
               VTXT.write(layer.opacities.by_flat_index(i));
            }
            VTXT.close();
         }
      }
      if (!this->textures.empty()) {
         auto& VTEX = record.open_next_subrecord('VTEX');
         for (auto& ref : this->textures) {
            VTEX.write(ref);
         }
         VTEX.close();
      }
      //
      if (!this->mpcd.empty()) {
         //
         // We can't currently generate MPCD data, so let's not save it.
         //
         detailed_notice warning;
         warning.code = notice_code::havok_data_is_not_supported_here;
         warning.set_cause_form(this->stub);
         warning.set_cause_subrecord('MPCD');
         intfc.log_save_warning(warning);
      }
      return true;
   }
   void Landscape::_clear_impl() noexcept {
      clear_form_reference_list(this->textures, *this);
      //
      for (auto& ref : this->default_quad_textures)
         ref.set(*this, nullptr);
      //
      for (auto& list : this->alpha_layers_by_quad) {
         for(auto& layer : list)
            layer.texture.set(*this, nullptr);
         list.clear();
      }
      //
      this->land_flags = land_flag::all_common_flags;
      for (auto& e : this->heightmap.heights.list())
         e = 0.0F;
      for (auto& e : this->heightmap.normals.list())
         e = { 0, 0, 1.0F };
      for (auto& e : this->heightmap.colors.list())
         e = { 255, 255, 255 };
      this->mpcd.clear();
   }
   void Landscape::_sever_outbound_references_impl(form_stub& other) noexcept {
      remove_form_from_reference_list(this->textures, other, *this);
      for (auto& ref : this->default_quad_textures)
         ref.clear_if(*this, other);
      for(auto& list : this->alpha_layers_by_quad)
         for (auto& layer : list)
            layer.texture.clear_if(*this, other);
   }
}