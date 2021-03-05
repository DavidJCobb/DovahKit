#include "Note.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Note::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      this->type = (note_type)0;
      //
      bool content_loaded = false;
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MODL':
            case 'MODT':
               this->model.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'DATA':
               {
                  auto prior = this->type;
                  this->type = (note_type)0;
                  subrecord.read(this->type);
                  if (this->type != prior && content_loaded) {
                     content_loaded = false;
                     this->content.sound.unmanaged_set(nullptr);
                     this->content.speaker.unmanaged_set(nullptr);
                     this->content.topic.unmanaged_set(nullptr);
                     this->content.text.reset();
                     this->content.image.clear();
                  }
               }
               break;
            case 'ONAM':
               if (subrecord.read(form_id)) {
                  this->owning_quests.push_back(form_id);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::quest, this->stub, form_id)
                  );
               }
               break;
            case 'XNAM':
               if (this->type != note_type::image) {
                  detailed_notice warning;
                  warning.code = notice_code::non_texture_note_includes_texture_path;
                  warning.set_cause_form(this->stub);
                  intfc.log_load_warning(warning);
                  break;
               }
               subrecord.to_string(this->content.image);
               break;
            case 'YNAM':
               if (subrecord.read(this->take_sound)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->take_sound)
                  );
               }
               break;
            case 'ZNAM':
               if (subrecord.read(this->drop_sound)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->drop_sound)
                  );
               }
               break;
            default:
               {
                  bool handled = false;
                  switch (this->type) {
                     //
                     // This is how Bethesda does it, though they don't react to entirely unknown 
                     // subrecords and so don't need the (handled) bool.
                     //
                     case note_type::image:
                        switch (subrecord.signature()) {
                           case 'ICON': // this handler should never actually run
                              handled = true;
                              subrecord.to_string(this->content.image);
                              break;
                        }
                        break;
                     case note_type::sound:
                        switch (subrecord.signature()) {
                           case 'SNAM':
                              handled = true;
                              if (subrecord.read(this->content.sound)) {
                                 intfc.log_load_warning(
                                    detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->content.sound)
                                 );
                              }
                              break;
                        }
                        break;
                     case note_type::text:
                        switch (subrecord.signature()) {
                           case 'TNAM':
                              handled = true;
                              subrecord.to_string(this->content.text);
                              break;
                        }
                        break;
                     case note_type::voice:
                        switch (subrecord.signature()) {
                           case 'SNAM':
                              handled = true;
                              if (subrecord.read(this->content.speaker)) {
                                 intfc.log_load_warning(
                                    detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::actor_base, this->stub, this->content.speaker)
                                 );
                              }
                              break;
                           case 'TNAM':
                              handled = true;
                              if (subrecord.read(this->content.topic)) {
                                 intfc.log_load_warning(
                                    detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::topic, this->stub, this->content.topic)
                                 );
                              }
                              break;
                        }
                        break;
                  }
                  if (handled) {
                     content_loaded = true;
                  }
                  if (!handled) {
                     intfc.log_load_warning(
                        detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
                     );
                  }
               }
               break;
         }
      }
   }
   /*static*/ void Note::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
   bool Note::_clone_impl(Form* out) const noexcept {
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
   bool Note::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
   void Note::_clear_impl() noexcept {
      this->model.clear();
      this->script_data.clear(*this);
      this->drop_sound.set(*this, nullptr);
      this->take_sound.set(*this, nullptr);
      clear_form_reference_list(this->owning_quests, *this);
      this->content.sound.set(*this, nullptr);
      this->content.speaker.set(*this, nullptr);
      this->content.topic.set(*this, nullptr);
   }
   void Note::_sever_outbound_references_impl(form_stub& other) noexcept {
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