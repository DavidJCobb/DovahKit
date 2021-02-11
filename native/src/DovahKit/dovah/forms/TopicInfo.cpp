#include "TopicInfo.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void TopicInfo::response::clear(TopicInfo& owner) {
      this->emotion = decltype(this->emotion)();
      this->unused = 0;
      this->sound.set(owner, nullptr);
      this->flags = 0;
      this->text.reset();
      this->script_notes.reset();
      this->edits.reset();
      this->idles.speaker.set(owner, nullptr);
      this->idles.listener.set(owner, nullptr);
   }
   void TopicInfo::response::clone_from(const response& other, loaded_forms::Form& my_owner) {
      this->emotion = other.emotion;
      this->unused  = other.unused;
      this->response_number = other.response_number;
      this->sound.set(my_owner, other.sound);
      this->flags = other.flags;
      this->text = other.text;
      this->script_notes = other.script_notes;
      this->edits = other.edits;
      this->idles.speaker.set(my_owner, other.idles.speaker);
      this->idles.listener.set(my_owner, other.idles.listener);
   }
   void TopicInfo::response::save(tes_file_writing::record& record, load_order_interfaces::form_save& intfc, uint8_t response_number) {
      auto& TRDT = record.open_next_subrecord('TRDT');
      TRDT.write(this->emotion.type);
      TRDT.write(this->emotion.value);
      TRDT.write(this->unused);
      TRDT.write(response_number);
      TRDT.skip_bytes(3);
      TRDT.write(this->sound);
      TRDT.write(this->flags);
      TRDT.skip_bytes(3);
      TRDT.close();
      auto& NAM1 = record.open_next_subrecord('NAM1');
      NAM1.write(this->text);
      NAM1.close();
      auto& NAM2 = record.open_next_subrecord('NAM2');
      NAM2.write(this->script_notes);
      NAM2.close();
      auto& NAM3 = record.open_next_subrecord('NAM3');
      NAM3.write(this->edits);
      NAM3.close();
      record.write_formID_subrecord('SNAM', this->idles.speaker, true);
      record.write_formID_subrecord('LNAM', this->idles.listener, true);
   }
   void TopicInfo::response::sever_outbound_references_to(TopicInfo& owner, form_stub& target) {
      this->sound.clear_if(owner, target);
      this->idles.speaker.clear_if(owner, target);
      this->idles.listener.clear_if(owner, target);
   }

   void TopicInfo::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      // Response data is only loaded for the winning record. The game doesn't load it during 
      // start-up, but rather on-demand, from a function that retrieves the info's last seen 
      // record. Even then, NAM2 and NAM3 are never loaded.
      //
      // All other data is loaded normally. However, TESTopicInfo::ClearData does not clear 
      // any data if the last-seen record (i.e. the one before the new override) was flagged 
      // as "partial." This means that if an INFO override is flagged as partial, its data 
      // (aside from response data, as mentioned) will be coalesced with the immediate next 
      // override to load. The reason for this behavior, and its potential uses, are unclear.
      //
      // This means that we need to load data and generate use info based on:
      //
      //  - The winning record
      //
      //  - Any unbroken series of partial-flagged files immediately before it
      //
      // The obvious approach would be to just blindly load all data from partial and winning 
      // records, and call (TopicInfo::clear) if the previous record wasn't partial, but that 
      // won't work because the (clear) function assumes that we're actually trying to edit 
      // the form and will bidirectionally sever use info... the use info that we generated 
      // in advance of loading using (TopicInfo::generate_use_info), which only generates use 
      // info for the data that we'll actually have after we load. So we'll be trying to sever 
      // uses that don't exist *and* more likely than not severing uses that we should keep.
      //
      // Nope. What we have to do is just not even load data unless we know we'll be keeping 
      // that data.
      //
      bool partial = (record.flags() & tes_file_record_header::flag::partial); // intfc.is_partial_record is only true for normal uses i.e. parent forms
      if (!intfc.is_winning_record && !partial)
         return;
      if (auto* file = intfc.current_file) {
         auto size = stub.source_file_count();
         auto here = stub.index_of_file(file);
         assert(here >= 0);
         int16_t i = size - 2;
         for (int16_t i = size - 2; i > here; --i) {
            auto* info = stub.get_source_file_info(i);
            if (!(info->flags & tes_file_record_header::flag::partial))
               return;
         }
      }
      //
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               if (subrecord.is_in_bounds(8)) {
                  float time;
                  subrecord.skip_bytes(2);
                  subrecord.unchecked_read(this->info_flags);
                  subrecord.unchecked_read(time);
                  time *= 65535.0F;
                  this->days_until_reset = time;
               }
               break;
            case 'ENAM':
               if (subrecord.is_in_bounds(8)) {
                  uint16_t time_until_reset; // 0xFFFF = 1 day
                  subrecord.unchecked_read(this->info_flags);
                  subrecord.unchecked_read(time_until_reset);
                  this->days_until_reset = (float)time_until_reset / 0xFFFF;
               }
               break;
            case 'TLCT':
               if (subrecord.read(formID)) {
                  auto& list = partial ? this->link_to.locked : this->link_to.normal;
                  list.push_back(formID);
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::topic, this->stub, formID)
                  );
               }
               break;
            case 'DNAM':
               if (subrecord.read(this->use_shared_info)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::topic_info, this->stub, this->use_shared_info)
                  );
               }
               break;
            case 'TPIC':
               if (subrecord.read(this->topic)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::topic, this->stub, this->topic)
                  );
               }
               break;
            case 'RNAM':
               if (subrecord.read(this->override_topic_text)) {
                  this->load_flags |= load_flag::has_topic_text_override;
               }
               break;
            case 'TWAT':
               if (subrecord.read(this->walk_away_topic)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::topic, this->stub, this->walk_away_topic)
                  );
               }
               break;
            case 'ANAM':
               if (subrecord.read(this->speaker)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::actor_base, this->stub, this->speaker)
                  );
               }
               break;
            case 'ONAM':
               if (subrecord.read(this->audio_override_output)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::sound_output_model, this->stub, this->audio_override_output)
                  );
               }
               break;
            case 'TRDT':
               if (!partial) {
                  auto& r = this->responses.emplace_back();
                  subrecord.read(r.emotion.type);
                  subrecord.read(r.emotion.value);
                  subrecord.read(r.unused);
                  subrecord.read(r.response_number);
                  subrecord.skip_bytes(3);
                  if (subrecord.read(r.sound)) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::sound_descriptor, this->stub, r.sound)
                     );
                  }
                  subrecord.read(r.flags);
                  subrecord.skip_bytes(3);
               }
               break;
            case 'NAM1':
               if (!partial && !this->responses.empty()) {
                  subrecord.read(this->responses.back().text);
               }
               break;
            case 'NAM2':
               if (!partial && !this->responses.empty()) {
                  subrecord.read(this->responses.back().script_notes);
               }
               break;
            case 'NAM3':
               if (!partial && !this->responses.empty()) {
                  subrecord.read(this->responses.back().edits);
               }
               break;
            case 'LNAM':
               if (!partial && !this->responses.empty()) {
                  auto& form = this->responses.back().idles.listener;
                  if (subrecord.read(form)) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::idle, this->stub, form)
                     );
                  }
               }
               break;
            case 'SNAM':
               if (!partial && !this->responses.empty()) {
                  auto& form = this->responses.back().idles.speaker;
                  if (subrecord.read(form)) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::idle, this->stub, form)
                     );
                  }
               }
               break;
            case 'CTDA':
               {
                  auto& list = partial ? this->conditions.locked : this->conditions.normal;
                  list.emplace_back().read(subrecord.get_containing_record(), intfc);
               }
               break;
            case 'OBND':
               this->object_bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void TopicInfo::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!(uib.last_record_flags() & tes_file_record_header::flag::partial)) {
         uib.clear_all_prior_use_info();
      }
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            //case 'PNAM': // previous-sibling topicinfo
            case 'TCLT': // follow-up topics
            case 'DNAM': // sharedinfo to inherit from
            case 'ANAM': // speaker
            case 'TWAT': // walk away topic
            case 'ONAM': // audio output override
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'DATA': // metadata (old)
            case 'ENAM': // metadata (new)
            case 'CNAM': // favor level
            case 'SCHR': // DEPRECATED: ObScript header
            case 'SCDA': // DEPRECATED: ObScript compiled code
            case 'SCTX': // DEPRECATED: ObScript source code
            case 'SCRO': // DEPRECATED: ObScript ObjectReference
            case 'SCRV': // DEPRECATED: ObScript ObjectReference variable
            case 'QNAM': // DEPRECATED: ObScript
            case 'NEXT': // ObScript separator
            case 'RNAM': // override topic text
               break;
            //
            // Response data only loads for the winning record:
            //
            case 'TRDT': // response header
               if (!uib.is_final_file())
                  break;
               subrecord.skip_bytes(16); // emotion type (4) and value (4); unused bytes (4); response number (1); padding bytes (3)
               if (subrecord.read(formID)) // sound
                  uib.add_outbound_reference(formID);
               break;
            case 'NAM1': // response text
            case 'NAM2': // response script notes
            case 'NAM3': // response edits
               break;
            case 'SNAM': // response speaker idle anim
            case 'LNAM': // response listener idle anim
               if (!uib.is_final_file())
                  break;
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
         }
      }
   }
   /*virtual*/ bool TopicInfo::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (TopicInfo*)out;
      //
      bool committing_to_self = (&out->stub == &this->stub) && this->is_working_copy;
      //
      copy->info_flags = this->info_flags;
      copy->load_flags = this->load_flags;
      copy->favor_level = this->favor_level;
      copy->days_until_reset = this->days_until_reset;
      copy->speaker.set(*copy, this->speaker);
      copy->topic.set(*copy, this->topic);
      copy->walk_away_topic.set(*copy, this->walk_away_topic);
      copy->use_shared_info.set(*copy, this->use_shared_info);
      copy->audio_override_output.set(*copy, this->audio_override_output);
      //
      if (committing_to_self) {
         copy->link_to.locked.reserve(this->link_to.locked.size());
         for (auto& id : this->link_to.locked)
            copy->link_to.locked.emplace_back().set(*copy, id);
         copy->link_to.normal.reserve(this->link_to.normal.size());
         for (auto& id : this->link_to.normal)
            copy->link_to.normal.emplace_back().set(*copy, id);
      } else {
         copy->link_to.normal.reserve(this->link_to.locked.size() + this->link_to.normal.size());
         for (auto& id : this->link_to.locked)
            copy->link_to.normal.emplace_back().set(*copy, id);
         for (auto& id : this->link_to.normal)
            copy->link_to.normal.emplace_back().set(*copy, id);
      }
      //
      if (committing_to_self) {
         copy->conditions.locked.reserve(this->conditions.locked.size());
         for (auto& cnd : this->conditions.locked)
            copy->conditions.locked.emplace_back().clone_from(cnd, *copy);
         copy->conditions.normal.reserve(this->conditions.normal.size());
         for (auto& cnd : this->conditions.normal)
            copy->conditions.normal.emplace_back().clone_from(cnd, *copy);
      } else {
         copy->conditions.normal.reserve(this->conditions.locked.size() + this->conditions.normal.size());
         for (auto& cnd : this->conditions.locked)
            copy->conditions.normal.emplace_back().clone_from(cnd, *copy);
         for (auto& cnd : this->conditions.normal)
            copy->conditions.normal.emplace_back().clone_from(cnd, *copy);
      }
      //
      copy->responses.reserve(this->responses.size());
      for (auto& r : this->responses)
         copy->responses.emplace_back().clone_from(r, *copy);
      //
      copy->override_topic_text = this->override_topic_text;
      copy->object_bounds = this->object_bounds;
      copy->script_data.clone_from(this->script_data, *copy);
      //
      return true;
   }
   /*virtual*/ bool TopicInfo::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (this->info_flags || this->days_until_reset) {
         auto& ENAM = record.open_next_subrecord('ENAM');
         ENAM.write(this->info_flags);
         uint16_t time = 0;
         if (this->days_until_reset >= 1.0F) {
            time = 0xFFFF;
         } else if (this->days_until_reset > 0.0F) {
            time = this->days_until_reset * 0xFFFF;
         }
         ENAM.write(time);
         ENAM.close();
      }
      if (const auto* prev = intfc.get_previous_child()) {
         record.write_formID_subrecord('PNAM', prev, true);
      }
      auto& CNAM = record.open_next_subrecord('CNAM');
      CNAM.write(this->favor_level);
      CNAM.close();
      for (auto& id : this->link_to.normal)
         record.write_formID_subrecord('TCLT', id, true);
      record.write_formID_subrecord('DNAM', this->use_shared_info, true);
      //
      if (auto size = this->responses.size()) {
         //
         // TODO: Does the game break if there are more than 256 responses, given that the 
         // responses' cached indices only go up to 255?
         //
         for (size_t i = 0; i < size; ++i) {
            this->responses[i].save(record, intfc, i);
         }
      }
      //
      for (auto& cnd : this->conditions.normal)
         cnd.save(record, intfc);
      if (!this->override_topic_text.empty()) {
         auto& RNAM = record.open_next_subrecord('RNAM');
         RNAM.write(this->override_topic_text);
         RNAM.close();
      }
      record.write_formID_subrecord('ANAM', this->speaker, true);
      record.write_formID_subrecord('TWAT', this->walk_away_topic, true);
      record.write_formID_subrecord('ONAM', this->audio_override_output, true);
      //
      return true;
   }
   /*virtual*/ void TopicInfo::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->speaker.clear_if(*this, other);
      this->topic.clear_if(*this, other);
      this->walk_away_topic.clear_if(*this, other);
      this->use_shared_info.clear_if(*this, other);
      this->audio_override_output.clear_if(*this, other);
      //
      remove_form_from_reference_list(this->link_to.locked, other, *this);
      remove_form_from_reference_list(this->link_to.normal, other, *this);
      //
      for (auto& cnd : this->conditions.locked)
         cnd.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions.normal)
         cnd.sever_outbound_references_to(other, *this);
      //
      for (auto& r : this->responses)
         r.sever_outbound_references_to(*this, other);
      //
      this->script_data.sever_outbound_references_to(other, *this);
   }
   /*virtual*/ void TopicInfo::_clear_impl() noexcept {
      this->info_flags = 0;
      this->load_flags = 0;
      this->favor_level = favor_level_t::none;
      this->days_until_reset = 0.0F;
      this->speaker.set(*this, nullptr);
      this->topic.set(*this, nullptr);
      this->walk_away_topic.set(*this, nullptr);
      this->use_shared_info.set(*this, nullptr);
      this->audio_override_output.set(*this, nullptr);
      //
      #if _DEBUG
         if (!this->link_to.locked.empty())
            //
            // It shouldn't be possible to perform any operation which would lead to 
            // the "locked" list being modified.
            //
            __debugbreak();
      #endif
      for (auto& id : this->link_to.locked)
         id.set(*this, nullptr);
      for (auto& id : this->link_to.normal)
         id.set(*this, nullptr);
      this->link_to.locked.clear();
      this->link_to.normal.clear();
      //
      #if _DEBUG
         if (!this->conditions.locked.empty())
            //
            // It shouldn't be possible to perform any operation which would lead to 
            // the "locked" list being modified.
            //
            __debugbreak();
      #endif
      for (auto& cnd : this->conditions.locked)
         cnd.clear(*this);
      for (auto& cnd : this->conditions.normal)
         cnd.clear(*this);
      this->conditions.locked.clear();
      this->conditions.normal.clear();
      //
      for (auto& r : this->responses)
         r.clear(*this);
      this->responses.clear();
      //
      this->override_topic_text.reset();
      this->object_bounds.clear();
      this->script_data.clear(*this);
   }
}