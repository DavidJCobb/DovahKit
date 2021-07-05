#include "Landscape.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Landscape::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      this->land_flags = 0;
      //
      form_reference_t form_id;
      bool alpha_layer_pending_data = false;
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
                  this->heightmap.colors.list[i] = { r, g, b };
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
                     this->heightmap.heights.list[i] = base_offset + span_offset;
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
                  auto& vec = this->heightmap.normals.list[i];
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
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::land_texture, this->stub, ref)
                  );
                  //
                  uint8_t quad;
                  if (subrecord.read(quad)) {
                     if (quad < 4)
                        this->default_quad_textures[quad] = ref;
                     else {
                        detailed_notice warning;
                        warning.type    = detailed_notice::notice_type::warning;
                        warning.context = detailed_notice::notice_context::on_demand_form_load;
                        warning.code    = notice_code::invalid_landscape_quad_index;
                        warning.set_cause_form(this->stub);
                        warning.set_cause_subrecord(subrecord.signature());
                        if (ref)
                           warning.add_relevant_form(*ref.get_form_stub());
                        warning.extra_integers[0] = quad;
                        intfc.log_load_warning(warning);
                     }
                  }
                  subrecord.skip_bytes(3); // BTXT and ATXT have the same header, but BTXT doesn't use the layer index. advise writing a -1 layer index when saving
               }
               break;
            case 'ATXT':
               {
                  alpha_layer layer;
                  subrecord.read(layer.texture);
                  subrecord.read(layer.quad);
                  subrecord.skip_bytes(1);
                  subrecord.read(layer.layer);
                  if (layer.quad > 3) {
                     detailed_notice warning;
                     warning.type    = detailed_notice::notice_type::warning;
                     warning.context = detailed_notice::notice_context::on_demand_form_load;
                     warning.code    = notice_code::invalid_landscape_quad_index;
                     warning.set_cause_form(this->stub);
                     warning.set_cause_subrecord(subrecord.signature());
                     if (layer.texture)
                        warning.add_relevant_form(*layer.texture.get_form_stub());
                     warning.extra_integers[0] = layer.quad;
                     intfc.log_load_warning(warning);
                     //
                     // Discard anything that would go into a bad quad. Excess layers are sensible to keep 
                     // around within an editor, because those can arise from a user clumsily painting a 
                     // landscape, and the user may want to edit them in a sensible way; however, excess 
                     // quads can only be garbage data.
                     //
                     break;
                  }
                  if (layer.layer > 5) {
                     detailed_notice warning;
                     warning.type    = detailed_notice::notice_type::warning;
                     warning.context = detailed_notice::notice_context::on_demand_form_load;
                     warning.code    = notice_code::landscape_quads_can_only_have_six_layers;
                     warning.set_cause_form(this->stub);
                     warning.set_cause_subrecord(subrecord.signature());
                     if (layer.texture)
                        warning.add_relevant_form(*layer.texture.get_form_stub());
                     warning.extra_integers[0] = layer.quad;
                     warning.extra_integers[1] = layer.layer;
                     intfc.log_load_warning(warning);
                  }
                  //
                  this->alpha_layers.push_back(layer);
                  alpha_layer_pending_data = (layer.layer >= 0);
               }
               break;
            case 'VTXT':
               if (subrecord.size() & 7) { // the game skips VTXT subrecords with an "uneven" length
                  break;
               }
               if (!alpha_layer_pending_data) { // the game loads only the first VTXT it sees for an ATXT
                  //
                  // For the curious: the game keeps two int32_ts on the stack, one for the last ATXT quad and 
                  // one for the last ATXT layer. VTXT skips its  content if either is -1, and sets them to -1 
                  // after being processed either way. Bethesda uses fixed-size arrays rather than vectors, so 
                  // they can't just use vector::empty.
                  //
                  break;
               }
               assert(!this->alpha_layers.empty());
               if (subrecord.is_in_bounds(8)) {
                  auto& layer = this->alpha_layers.back();
                  if (layer.layer < 0)
                     break;
                  auto& entry = layer.alpha.emplace_back();
                  subrecord.unchecked_read(entry.index);
                  subrecord.skip_bytes(2);
                  subrecord.unchecked_read(entry.value);
                  //
                  alpha_layer_pending_data = false;
               }
               break;
            case 'VTEX':
               while (subrecord.is_in_bounds(4)) {
                  auto& ref = this->textures.emplace_back();
                  subrecord.unchecked_read(ref);
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::land_texture, this->stub, ref)
                  );
               }
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
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
               while (subrecord.is_in_bounds(0)) {
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
      if (out->formType != form_type)
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
      size_t size = this->alpha_layers.size();
      assert(copy->alpha_layers.empty());
      copy->alpha_layers.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src = this->alpha_layers[i];
         auto& dst = copy->alpha_layers[i];
         dst.texture.set(*copy, src.texture);
         dst.quad  = src.quad;
         dst.layer = src.layer;
         dst.alpha = src.alpha;
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
         auto& vec = this->heightmap.normals.list[i];
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
         auto& list = this->heightmap.heights.list;
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
         float base_offset = floor(this->heightmap.heights.list[0] / 8.0F);
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
         #if !_DEBUG
            static_assert(false, "Test this code before you ship anything! Even a JavaScript simulation is better than nothing!");
         #endif
      }
      VHGT.close();
      //
      auto& VCLR = record.open_next_subrecord('VCLR');
      for (int i = 0; i < total_vertex_count; ++i) {
         auto& color = this->heightmap.colors.list[i];
         VCLR.write(color.r);
         VCLR.write(color.g);
         VCLR.write(color.b);
      }
      VCLR.close();
      //
      {
         auto& list = this->default_quad_textures;
         for (int i = 0; i < list.size(); ++i) {
            auto& ref  = list[i];
            if (ref == nullptr)
               continue;
            auto& BTXT = record.open_next_subrecord('BTXT');
            BTXT.write(ref);
            BTXT.write(uint8_t(i));
            BTXT.skip_bytes(1);
            BTXT.write(int16_t(-1));
            BTXT.close();
         }
      }
      for (auto& layer : this->alpha_layers) {
         if (layer.quad > 3) {
            detailed_notice error;
            error.code = notice_code::invalid_landscape_quad_index;
            error.set_cause_form(this->stub);
            error.set_cause_subrecord('ATXT');
            if (layer.texture)
               error.add_relevant_form(*layer.texture.get_form_stub());
            error.extra_integers[0] = layer.quad;
            intfc.set_save_error(error);
            //
            return false;
         }
         auto& ATXT = record.open_next_subrecord('ATXT');
         ATXT.write(layer.texture);
         ATXT.write(layer.quad);
         ATXT.skip_bytes(1);
         ATXT.write(layer.layer);
         ATXT.close();
         //
         auto& VTXT = record.open_next_subrecord('VTXT');
         for (auto& entry : layer.alpha) {
            VTXT.write(entry.index);
            VTXT.skip_bytes(2);
            VTXT.write(entry.value);
         }
         VTXT.close();
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
      for (auto& layer : this->alpha_layers) {
         layer.texture.set(*this, nullptr);
      }
      this->alpha_layers.clear();
      //
      this->land_flags = land_flag::all_common_flags;
      for (auto& e : this->heightmap.heights.list)
         e = 0.0F;
      for (auto& e : this->heightmap.normals.list)
         e = { 0, 0, 1.0F };
      for (auto& e : this->heightmap.colors.list)
         e = { 255, 255, 255 };
      this->mpcd.clear();
   }
   void Landscape::_sever_outbound_references_impl(form_stub& other) noexcept {
      remove_form_from_reference_list(this->textures, other, *this);
      for (auto& ref : this->default_quad_textures)
         ref.clear_if(*this, other);
      for (auto& layer : this->alpha_layers)
         layer.texture.clear_if(*this, other);
   }
}