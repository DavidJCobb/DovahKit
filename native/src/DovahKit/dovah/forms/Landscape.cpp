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
               if (!subrecord.is_in_bounds(sizeof(float) + total_vertex_count))
                  break;
               subrecord.unchecked_read(this->heightmap.base);
               for (int i = 0; i < total_vertex_count; ++i) {
                  subrecord.unchecked_read(this->heightmap.heights.list[i]);
               }
               subrecord.skip_bytes(3); // padding
               break;
            case 'VNML': // vertex normals
               for (int i = 0; i < total_vertex_count; ++i) {
                  if (!subrecord.is_in_bounds(3))
                     break;
                  uint8_t x;
                  uint8_t y;
                  uint8_t z;
                  subrecord.unchecked_read(x);
                  subrecord.unchecked_read(y);
                  subrecord.unchecked_read(z);
                  //
                  auto& vec = this->heightmap.normals.list[i++];
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
                     static_assert(false, "TODO: When we write the save code, out-of-bounds quad indices should result in a save error and should fail the save operation.");
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
      form_id_t take_sound;
      form_id_t drop_sound;
      form_id_t content_actor;
      form_id_t content_sound;
      form_id_t content_topic;
      form_id_t form_id;
      note_type type = (note_type)0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               {
                  auto prior = type;
                  type = (note_type)0;
                  subrecord.read(type);
                  if (type != prior) {
                     content_actor = 0;
                     content_sound = 0;
                     content_topic = 0;
                  }
               }
               break;
            case 'ONAM':
               if (subrecord.read(form_id))
                  uib.add_outbound_reference(form_id);
               break;
            case 'XNAM': // texture content (alternate way to specify)
               break;
            case 'YNAM':
               subrecord.read(take_sound);
               break;
            case 'ZNAM':
               subrecord.read(drop_sound);
               break;
            default:
               switch (type) {
                  //
                  // This is how Bethesda does it, though they don't react to entirely unknown 
                  // subrecords and so don't need the (handled) bool.
                  //
                  case note_type::image:
                     switch (subrecord.signature()) {
                        case 'ICON':
                           break;
                     }
                     break;
                  case note_type::sound:
                     switch (subrecord.signature()) {
                        case 'SNAM':
                           subrecord.read(content_sound);
                           break;
                     }
                     break;
                  case note_type::text:
                     switch (subrecord.signature()) {
                        case 'TNAM':
                           break;
                     }
                     break;
                  case note_type::voice:
                     switch (subrecord.signature()) {
                        case 'SNAM':
                           subrecord.read(content_actor);
                           break;
                        case 'TNAM':
                           subrecord.read(content_topic);
                           break;
                     }
                     break;
               }
               break;
         }
      }
      uib.add_outbound_reference(take_sound);
      uib.add_outbound_reference(drop_sound);
      uib.add_outbound_reference(content_actor);
      uib.add_outbound_reference(content_sound);
      uib.add_outbound_reference(content_topic);
      uib.add_outbound_reference(form_id);
   }
   bool Landscape::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Note*)out;
      //
      copy->model.clone_from(this->model);
      copy->script_data.clear(*copy);
      copy->drop_sound.set(*copy, this->drop_sound);
      copy->take_sound.set(*copy, this->take_sound);
      copy_form_reference_list(*copy, copy->owning_quests, this->owning_quests);
      copy->content.sound.set(*copy, this->content.sound);
      copy->content.speaker.set(*copy, this->content.speaker);
      copy->content.topic.set(*copy, this->content.topic);
      //
      return true;
   }
   bool Landscape::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT');
      record.write_string_subrecord('ICON', this->icon);
      record.write_formID_subrecord('YNAM', this->take_sound, true);
      record.write_formID_subrecord('ZNAM', this->drop_sound, true);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->type);
      DATA.close();
      for (auto& id : this->owning_quests)
         record.write_formID_subrecord('ONAM', id, true);
      if (this->type == note_type::image)
         record.write_string_subrecord('XNAM', this->content.image);
      else if (this->type == note_type::text) {
         auto& TNAM = record.open_next_subrecord('TNAM');
         TNAM.write(this->content.text);
         TNAM.close();
      } else if (this->type == note_type::sound) {
         record.write_formID_subrecord('SNAM', this->content.sound);
      } else if (this->type == note_type::voice) {
         record.write_formID_subrecord('TNAM', this->content.topic);
         record.write_formID_subrecord('SNAM', this->content.speaker);
      }
      return true;
   }
   void Landscape::_clear_impl() noexcept {
      this->model.clear();
      this->script_data.clear(*this);
      this->drop_sound.set(*this, nullptr);
      this->take_sound.set(*this, nullptr);
      clear_form_reference_list(this->owning_quests, *this);
      this->content.sound.set(*this, nullptr);
      this->content.speaker.set(*this, nullptr);
      this->content.topic.set(*this, nullptr);
   }
   void Landscape::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->drop_sound.clear_if(*this, other);
      this->take_sound.clear_if(*this, other);
      remove_form_from_reference_list(this->owning_quests, other, *this);
      this->content.sound.clear_if(*this, other);
      this->content.speaker.clear_if(*this, other);
      this->content.topic.clear_if(*this, other);
   }
}