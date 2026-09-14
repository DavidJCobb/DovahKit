#include "MusicTrack.h"
#include <bitset>
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/music_track/invalid_track_type.h"
#include "../notices/form_load_warnings/by_form_type/music_track/too_many_palette_layers.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::music_track;
   }
}

namespace {
   enum class serialized_track_type : uint32_t {
      palette = 0x23F678C3,
      single  = 0x6ED7E048,
      silent  = 0xA1A9C4D5,
   };
}

namespace dovah::loaded_forms {
   #pragma region MusicTrack::palette_data
      void MusicTrack::palette_data::set_all_tracks(MusicTrack& my_containing_form, const std::array<std::vector<form_reference_t>, max_palette_layer_count>& src_layers) {
         auto& dst_layers = this->tracks_by_layer;
         for (size_t i = 0; i < max_palette_layer_count; ++i) {
            const auto& src_layer = src_layers[i];
            auto&       dst_layer = dst_layers[i];
            copy_form_reference_list(my_containing_form, dst_layer, src_layer);
            //
            // Palettes' track sets are serialized as a single flat list, with null form IDs 
            // separating layers. This means that we can't allow nulls within a layer.
            //
            std::erase(dst_layer, nullptr);
         }
      }
      void MusicTrack::palette_data::set_all_tracks(MusicTrack& my_containing_form, const std::array<std::vector<form_stub*>, max_palette_layer_count>& src_layers) {
         auto& dst_layers = this->tracks_by_layer;
         for (size_t i = 0; i < max_palette_layer_count; ++i) {
            const auto& src_layer = src_layers[i];
            auto&       dst_layer = dst_layers[i];
            clear_form_reference_list(dst_layer, my_containing_form);
            for (auto& form : src_layer) {
               if (!form)
                  //
                  // Palettes' track sets are serialized as a single flat list, with null form 
                  // IDs separating layers. This means that we can't allow nulls within a layer.
                  //
                  continue;
               dst_layer.emplace_back().set(my_containing_form, form);
            }
            dst_layer.shrink_to_fit();
         }
      }
   #pragma endregion

   music_track_type MusicTrack::get_track_type() const {
      return (music_track_type)this->data.index();
   }
   void MusicTrack::set_track_type(music_track_type t) {
      if (this->get_track_type() == t)
         return;
      
      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         for(auto& layer : casted->tracks_by_layer)
            clear_form_reference_list(layer, *this);
      }
      switch (t) {
         case music_track_type::palette:
            this->data.emplace<palette_data>();
            break;
         case music_track_type::single:
            this->data.emplace<single_data>();
            break;
         case music_track_type::silent:
            this->data.emplace<silent_data>();
            break;
      }
   }

   void MusicTrack::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      size_t palette_layer_index = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'CNAM':
               {
                  uint32_t type;
                  if (subrecord.read(type)) {
                     //
                     // Supposedly, this value is a CRC32 hash of the internal class 
                     // name for a given music type. I haven't been able to verify 
                     // that yet.
                     //
                     switch ((serialized_track_type)type) {
                        case serialized_track_type::palette:
                           this->data.emplace<palette_data>();
                           palette_layer_index = 0;
                           break;
                        case serialized_track_type::single:
                           this->data.emplace<single_data>();
                           break;
                        case serialized_track_type::silent:
                           this->data.emplace<silent_data>();
                           break;
                        default:
                           {
                              specific_load_warnings::invalid_track_type notice(
                                 this->stub,
                                 type
                              );
                              intfc.log_load_warning(notice);
                           }
                           break;
                     }
                  }
               }
               break;
            case 'FLTV':
               if (std::holds_alternative<palette_data>(this->data)) {
                  auto& casted = std::get<palette_data>(this->data);
                  subrecord.read(casted.duration);
               } else if (std::holds_alternative<silent_data>(this->data)) {
                  auto& casted = std::get<silent_data>(this->data);
                  subrecord.read(casted.duration);
               }
               break;
            case 'DNAM':
               if (std::holds_alternative<palette_data>(this->data)) {
                  auto& casted = std::get<palette_data>(this->data);
                  subrecord.read(casted.fade_out);
               }
               break;
            case 'ANAM':
               if (std::holds_alternative<single_data>(this->data)) {
                  auto& casted = std::get<single_data>(this->data);
                  subrecord.read(casted.filenames.main);
               }
               break;
            case 'BNAM':
               if (std::holds_alternative<single_data>(this->data)) {
                  auto& casted = std::get<single_data>(this->data);
                  subrecord.read(casted.filenames.finale);
               }
               break;
            case 'FNAM':
               if (std::holds_alternative<single_data>(this->data)) {
                  auto& casted = std::get<single_data>(this->data);
                  casted.cue_points.resize(subrecord.size() / sizeof(float));
                  for (auto& item : casted.cue_points)
                     subrecord.unchecked_read(item);
               }
               break;
            case 'LNAM':
               if (std::holds_alternative<single_data>(this->data)) {
                  auto& casted = std::get<single_data>(this->data);
                  if (!casted.loop.has_value())
                     casted.loop.emplace();

                  auto& dst = casted.loop.value();
                  subrecord.read(dst.begin);
                  subrecord.read(dst.end);
                  subrecord.read(dst.count);
               }
               break;
            case 'CITC':
               {
                  uint32_t count;
                  if (subrecord.read(count))
                     this->conditions.reserve(count);
               }
               break;
            case 'CTDA':
               this->conditions.read_next(record, intfc);
               break;
            case 'SNAM':
               if (std::holds_alternative<palette_data>(this->data)) {
                  auto& casted = std::get<palette_data>(this->data);
                  while (subrecord.is_in_bounds(4)) {
                     form_reference_t ref;
                     if (!subrecord.read(ref))
                        break;
                     if (!ref) {
                        ++palette_layer_index;
                        continue;
                     }
                     if (palette_layer_index >= max_palette_layer_count) {
                        continue;
                     }
                     intfc.warn_if_ref_is_wrong_type(ref, form_type::music_track, subrecord.signature());
                     auto& layer = casted.tracks_by_layer[palette_layer_index];
                     layer.emplace_back().unmanaged_set(ref.get_form_stub());
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (palette_layer_index >= max_palette_layer_count) {
         specific_load_warnings::too_many_palette_layers notice(
            this->stub,
            palette_layer_index,
            max_palette_layer_count
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void MusicTrack::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      std::vector<form_id_t> palette_tracks;
      bool is_palette = false;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'CNAM':
               {
                  uint32_t type;
                  if (subrecord.read(type)) {
                     palette_tracks.clear();
                     if (type == 0x23F678C3)
                        is_palette = true;
                  }
               }
               break;
            case 'SNAM':
               do {
                  auto& id = palette_tracks.emplace_back();
                  if (!subrecord.read(id))
                     break;
               } while (true);
               break;
         }
      }
      if (is_palette) {
         for (auto id : palette_tracks)
            uib.add_outbound_reference(id);
      }
   }
   void MusicTrack::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MusicTrack*)out;

      copy->script_data.clone_from(this->script_data, *copy);

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      {
         auto& src = this->data;
         auto& dst = copy->data;
         if (std::holds_alternative<palette_data>(src)) {
            auto& src_data = std::get<palette_data>(src);
            auto& dst_data = dst.emplace<palette_data>();
            dst_data.duration = src_data.duration;
            dst_data.fade_out = src_data.fade_out;
            {
               auto& src_layers = src_data.tracks_by_layer;
               auto& dst_layers = dst_data.tracks_by_layer;
               for (size_t i = 0; i < max_palette_layer_count; ++i) {
                  copy_form_reference_list(*copy, dst_layers[i], src_layers[i]);
               }
            }
         } else {
            dst = src;
         }
      }
   }
   void MusicTrack::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         uint32_t type = 0;
         switch (this->data.index()) {
            case 0:
               type = (uint32_t)serialized_track_type::palette;
               break;
            case 1:
               type = (uint32_t)serialized_track_type::single;
               break;
            case 2:
               type = (uint32_t)serialized_track_type::silent;
               break;
         }
         subrecord.write(type);
         subrecord.close();
      }
      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         {
            auto& subrecord = record.open_next_subrecord('FLTV');
            subrecord.write(casted->duration);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('DNAM');
            subrecord.write(casted->fade_out);
            subrecord.close();
         }

      } else if (auto* casted = std::get_if<single_data>(&this->data)) {
         record.write_string_subrecord('ANAM', casted->filenames.main);
         record.write_string_subrecord('BNAM', casted->filenames.finale);
         {
            auto& list = casted->cue_points;
            if (!list.empty()) {
               auto& subrecord = record.open_next_subrecord('FNAM');
               for (auto& item : list)
                  subrecord.write(item);
               subrecord.close();
            }
         }
         if (casted->loop.has_value()) {
            auto& subrecord = record.open_next_subrecord('LNAM');
            subrecord.write(casted->loop->begin);
            subrecord.write(casted->loop->end);
            subrecord.write(casted->loop->count);
            subrecord.close();
         }
      } else if (auto* casted = std::get_if<silent_data>(&this->data)) {
         auto& subrecord = record.open_next_subrecord('FLTV');
         subrecord.write(casted->duration);
         subrecord.close();
      }
      if (!this->conditions.empty()) {
         auto& list = this->conditions;
         {
            auto& subrecord = record.open_next_subrecord('CITC');
            subrecord.write((uint32_t)list.size());
            subrecord.close();
         }
         for (auto& item : list)
            item.save(record, intfc);
      }
      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         const auto& layers = casted->tracks_by_layer;

         bool   has_non_empty_layers = false;
         size_t last_non_empty_layer = 0;
         for (size_t i = 0; i < layers.size(); ++i) {
            auto& layer = layers[i];
            for (auto& form : layer) {
               if (form) {
                  has_non_empty_layers = true;
                  last_non_empty_layer = i;
                  break;
               }
            }
         }
         if (has_non_empty_layers) {
            auto& subrecord = record.open_next_subrecord('SNAM');
            for (size_t i = 0; i <= last_non_empty_layer; ++i) {
               //
               // Palettes' track sets are serialized as a single flat list, with null 
               // form IDs separating layers. This means that we can't allow nulls 
               // within a layer, and it means we have to insert nulls between layers. 
               // At the same time, we should not have a trailing null after the last 
               // layer we save.
               //
               if (i > 0) {
                  subrecord.write((dovah::form_stub*)nullptr);
               }
               const auto& layer = layers[i];
               for (auto& form : layer)
                  if (form)
                     subrecord.write(form);
            }
            subrecord.close();
         }
      }
   }
   void MusicTrack::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);

      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         for(auto& layer : casted->tracks_by_layer)
            clear_form_reference_list(layer, *this);
      }
      this->data.emplace<palette_data>();
   }
   void MusicTrack::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         for (auto& layer : casted->tracks_by_layer)
            remove_form_from_reference_list(layer, other, *this);
      }
   }
}