#include "MusicTrack.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/music_track/invalid_track_type.h"

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
   void MusicTrack::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

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

                  float v;
                  if (subrecord.read(v))
                     casted.cue_points.push_back(v);
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
                  if (auto& form = casted.tracks.emplace_back(); subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::music_track, subrecord.signature());
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
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
               subrecord.read(palette_tracks.emplace_back());
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
         if (auto* casted = std::get_if<palette_data>(&dst)) {
            clear_form_reference_list(casted->tracks, *copy);
         }

         if (std::holds_alternative<palette_data>(src)) {
            auto& src_data = std::get<palette_data>(src);
            auto& dst_data = dst.emplace<palette_data>();
            dst_data.duration = src_data.duration;
            dst_data.fade_out = src_data.fade_out;
            copy_form_reference_list(*copy, dst_data.tracks, src_data.tracks);
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
         for (auto v : casted->cue_points) {
            auto& subrecord = record.open_next_subrecord('FNAM');
            subrecord.write(v);
            subrecord.close();
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
         for(auto& item : casted->tracks) {
            record.write_formID_subrecord('SNAM', item);
         }
      }
   }
   void MusicTrack::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);

      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         clear_form_reference_list(casted->tracks, *this);
      }
      this->data.emplace<palette_data>();
   }
   void MusicTrack::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      if (auto* casted = std::get_if<palette_data>(&this->data)) {
         remove_form_from_reference_list(casted->tracks, other, *this);
      }
   }
}