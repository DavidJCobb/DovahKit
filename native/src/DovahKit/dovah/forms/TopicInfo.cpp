#include "TopicInfo.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
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
                  this->link_to.push_back(formID);
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
                  if (subrecord.read(r.sound)) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), dovah::form_type::sound_descriptor, this->stub, r.sound)
                     );
                  }
                  subrecord.read(r.flags);
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
               this->conditions.emplace_back().read(subrecord.get_containing_record(), intfc);
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
}