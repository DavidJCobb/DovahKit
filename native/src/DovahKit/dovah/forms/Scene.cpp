#include "Scene.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/scene/invalid_scene_action_type.h"
#include "../notices/form_load_warnings/by_form_type/scene/scene_action_base_layout_incorrect.h"
#include "../notices/form_load_warnings/by_form_type/scene/scene_action_necessary_subrecord_missing.h"
#include "../notices/form_load_warnings/by_form_type/scene/scene_action_necessary_subrecord_unreadable.h"
#include "../notices/form_load_warnings/by_form_type/scene/scene_actor_subrecords_out_of_order.h"
#include "../notices/form_load_warnings/by_form_type/scene/unexpected_subrecord_in_scene_action.h"
#include "../notices/form_load_warnings/by_form_type/scene/unexpected_subrecord_in_scene_phase.h"
#include "../notices/form_load_warnings/by_form_type/scene/unterminated_scene_phase.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::scene;
   }
}

namespace dovah::loaded_forms {
   #pragma region Scene::phase
      bool Scene::phase::load(tes_record_reader& record, load_order_interfaces::form_load& intfc, size_t which) {
         bool saw_next = false;
         while (auto& subrecord = record.next_subrecord()) {
            switch (subrecord.signature()) {
               case 'HNAM':
                  //
                  // End marker.
                  //
                  return true;
               case 'NAM0':
                  subrecord.read(this->name);
                  break;
               case 'NEXT':
                  saw_next = true;
                  break;
               case 'CTDA':
                  if (saw_next) {
                     this->conditions.completion.read_next(record, intfc);
                  } else {
                     this->conditions.start.read_next(record, intfc);
                  }
                  break;
               case 'WNAM':
                  subrecord.read(this->editor_display_width);
                  break;
               case components::legacy_script::subrecord_signature_header:
               case components::legacy_script::subrecord_signature_compiled_data:
               case components::legacy_script::subrecord_signature_source_code:
               case components::legacy_script::subrecord_signature_quest:
               case components::legacy_script::subrecord_signature_ref_objects:
               case components::legacy_script::subrecord_signature_ref_variables:
                  break;
               default:
                  specific_load_warnings::unexpected_subrecord_in_scene_phase notice(
                     intfc.target_stub,
                     which,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
            }
         }
         //
         // No end-marker present.
         //
         return false;
      }
      /*static*/ void Scene::phase::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
         auto sub_uib  = uib.spawn_subordinate_on_stack();
         bool saw_next = false;
         while (auto& subrecord = record.next_subrecord()) {
            switch (subrecord.signature()) {
               case 'HNAM':
                  //
                  // Terminated scene phase. Commit uses.
                  //
                  sub_uib.commit();
                  return;
               case 'NAM0':
                  break;
               case 'NEXT':
                  break;
               case 'CTDA':
                  components::condition::generate_use_info(record, sub_uib);
                  break;
               case 'WNAM':
                  break;
               case components::legacy_script::subrecord_signature_header:
               case components::legacy_script::subrecord_signature_compiled_data:
               case components::legacy_script::subrecord_signature_source_code:
               case components::legacy_script::subrecord_signature_quest:
               case components::legacy_script::subrecord_signature_ref_objects:
               case components::legacy_script::subrecord_signature_ref_variables:
                  break;
               default:
                  break;
            }
         }
         //
         // Unterminated scene phase. Don't commit any uses.
         //
      }
      void Scene::phase::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         record.open_next_subrecord('HNAM').close(); // start marker
         if (!this->name.empty())
            record.write_string_subrecord('NAM0', this->name);
         for (auto& cnd : this->conditions.start)
            cnd.save(record, intfc);
         record.open_next_subrecord('NEXT').close();
         for (auto& cnd : this->conditions.completion)
            cnd.save(record, intfc);
         // TODO: legacy script (begin?)
         record.open_next_subrecord('NEXT').close();
         // TODO: legacy script (end?)
         {
            auto& subrecord = record.open_next_subrecord('WNAM');
            subrecord.write(this->editor_display_width);
            subrecord.close();
         }
         record.open_next_subrecord('HNAM').close(); // end marker
      }
      void Scene::phase::clear(loaded_forms::Form& my_containing_form) noexcept {
         this->name.clear();
         this->conditions.start.clear(my_containing_form);
         this->conditions.completion.clear(my_containing_form);
         this->editor_display_width = 32;
      }
      void Scene::phase::clone_from(const phase& src, loaded_forms::Form& my_containing_form) noexcept {
         this->name = src.name;
         {
            auto& dst = this->conditions.start;
            assert(dst.empty());
            dst.append_all_of(my_containing_form, src.conditions.start);
         }
         {
            auto& dst = this->conditions.completion;
            assert(dst.empty());
            dst.append_all_of(my_containing_form, src.conditions.completion);
         }
         this->editor_display_width = src.editor_display_width;
      }
      void Scene::phase::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
         for (auto& cnd : this->conditions.start)
            cnd.sever_outbound_references_to(target, my_containing_form);
         for (auto& cnd : this->conditions.completion)
            cnd.sever_outbound_references_to(target, my_containing_form);
      }
   #pragma endregion

   #pragma region Scene::action
      bool Scene::action::_load_base_properties(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         bool success = true;
         while (auto& subrecord = record.next_subrecord()) {
            bool done = false;
            switch (subrecord.signature()) {
               case 'NAM0':
                  subrecord.read(this->name);
                  break;
               case 'ALID':
                  success = subrecord.read(this->alias_id);
                  break;
               case 'INAM':
                  success = subrecord.read(this->action_id);
                  break;
               case 'FNAM':
                  subrecord.read(this->flags);
                  break;
               case 'SNAM':
                  success = subrecord.read(this->phase_indices.start);
                  break;
               case 'ENAM':
                  success = subrecord.read(this->phase_indices.end);
                  //
                  // We stop consuming BGSSceneAction subrecords when we see ENAM.
                  //
                  done = true;
                  break;
               default:
                  specific_load_warnings::scene_action_base_layout_incorrect notice(
                     intfc.target_stub,
                     this->action_id,
                     subrecord.signature(),
                     specific_load_warnings::scene_action_base_layout_incorrect::problem_type::unrecognized_subrecord
                  );
                  intfc.log_load_warning(notice);
                  return false;
            }
            if (!success) {
               specific_load_warnings::scene_action_base_layout_incorrect notice(
                  intfc.target_stub,
                  this->action_id,
                  subrecord.signature(),
                  specific_load_warnings::scene_action_base_layout_incorrect::problem_type::malformed_valid_subrecord
               );
               intfc.log_load_warning(notice);
               break;
            }
            if (done)
               break;
         }
         return success;
      }
      bool Scene::action::_load_data_for_dialogue(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         auto& action_data   = std::get<dialogue_data>(this->data);
         bool  has_necessary = false;
         while (auto& subrecord = record.next_subrecord()) {
            const auto signature = subrecord.signature();
            if (signature == 'ANAM') {
               break;
            }
            switch (signature) {
               case 'DATA':
                  if (auto& dst = action_data.topic; subrecord.read(dst)) {
                     has_necessary = true;
                     intfc.warn_if_ref_is_wrong_type(dst, form_type::topic, subrecord.signature());
                  } else {
                     specific_load_warnings::scene_action_necessary_subrecord_unreadable notice(
                        intfc.target_stub,
                        this->action_id,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                     return false;
                  }
                  break;
               case 'DEVA':
                  subrecord.read(action_data.emotion.value);
                  break;
               case 'DEMO':
                  subrecord.read(action_data.emotion.type);
                  break;
               case 'HTID':
                  subrecord.read(action_data.headtrack_alias_id);
                  break;
               case 'DMIN':
                  subrecord.read(action_data.looping.min);
                  break;
               case 'DMAX':
                  subrecord.read(action_data.looping.max);
                  break;
               default:
                  specific_load_warnings::unexpected_subrecord_in_scene_action notice(
                     intfc.target_stub,
                     this->action_id,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
            }
         }
         if (!has_necessary) {
            specific_load_warnings::scene_action_necessary_subrecord_missing notice(
               intfc.target_stub,
               this->action_id,
               'DATA'
            );
            intfc.log_load_warning(notice);
         }
         return has_necessary;
      }
      bool Scene::action::_load_data_for_package(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         auto& action_data   = std::get<package_data>(this->data);
         bool  has_necessary = false;
         while (auto& subrecord = record.next_subrecord()) {
            const auto signature = subrecord.signature();
            if (signature == 'ANAM') {
               break;
            }
            switch (signature) {
               case 'PNAM':
                  has_necessary = true;
                  {
                     form_reference_t id;
                     if (!subrecord.read(id)) {
                        specific_load_warnings::scene_action_necessary_subrecord_unreadable notice(
                           intfc.target_stub,
                           this->action_id,
                           subrecord.signature()
                        );
                        intfc.log_load_warning(notice);
                        return false;
                     }
                     if (id) {
                        intfc.warn_if_ref_is_wrong_type(id, form_type::package, subrecord.signature());
                        action_data.packages.push_back(id);
                     }
                  }
                  break;
               default:
                  specific_load_warnings::unexpected_subrecord_in_scene_action notice(
                     intfc.target_stub,
                     this->action_id,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
            }
         }
         //
         // Failed to find an ANAM or PNAM.
         //
         if (!has_necessary) {
            specific_load_warnings::scene_action_necessary_subrecord_missing notice(
               intfc.target_stub,
               this->action_id,
               'PNAM'
            );
            intfc.log_load_warning(notice);
         }
         return has_necessary;
      }
      bool Scene::action::_load_data_for_timer(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         auto& action_data   = std::get<timer_data>(this->data);
         bool  has_necessary = false;
         while (auto& subrecord = record.next_subrecord()) {
            const auto signature = subrecord.signature();
            if (signature == 'ANAM') {
               break;
            }
            switch (signature) {
               case 'SNAM':
                  has_necessary = true;
                  if (!subrecord.read(action_data.duration)) {
                     specific_load_warnings::scene_action_necessary_subrecord_unreadable notice(
                        intfc.target_stub,
                        this->action_id,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                     return false;
                  }
                  break;
               default:
                  specific_load_warnings::unexpected_subrecord_in_scene_action notice(
                     intfc.target_stub,
                     this->action_id,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
            }
         }
         //
         // Failed to find an ANAM or SNAM.
         //
         if (!has_necessary) {
            specific_load_warnings::scene_action_necessary_subrecord_missing notice(
               intfc.target_stub,
               this->action_id,
               'SNAM'
            );
            intfc.log_load_warning(notice);
         }
         return has_necessary;
      }

      bool Scene::action::load(tes_record_reader& record, load_order_interfaces::form_load& intfc, action_type type) {
         switch (type) {
            case action_type::dialogue:
               this->data.emplace<action::dialogue_data>();
               break;
            case action_type::package:
               this->data.emplace<action::package_data>();
               break;
            case action_type::timer:
               this->data.emplace<action::timer_data>();
               break;
            default:
               specific_load_warnings::invalid_scene_action_type notice(
                  intfc.target_stub,
                  (uint16_t)type
               );
               intfc.log_load_warning(notice);
               return false;
         }
         assert(this->type() == type);
         //
         // First, the BGSSceneAction subrecord:
         //
         bool base_succeeded = _load_base_properties(record, intfc);
         if (!base_succeeded) {
            return false;
         }
         //
         // Next, specific types:
         //
         switch (type) {
            case action_type::dialogue:
               //
               // BGSSceneActionDialogue
               //
               return _load_data_for_dialogue(record, intfc);
            case action_type::package:
               //
               // BGSSceneActionPackage
               //
               return _load_data_for_package(record, intfc);
            case action_type::timer:
               //
               // BGSSceneActionTimer
               //
               return _load_data_for_timer(record, intfc);
         }
         std::unreachable();
         return false;
      }

      /*static*/ void Scene::action::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, action_type type) {
         //
         // BGSSceneAction:
         //
         while (auto& subrecord = record.next_subrecord()) {
            bool done = false;
            switch (subrecord.signature()) {
               case 'NAM0':
                  break;
               case 'ALID':
                  if (!subrecord.skip_bytes(sizeof(alias_id)))
                     return;
                  break;
               case 'INAM':
                  if (!subrecord.skip_bytes(sizeof(action_id)))
                     return;
                  break;
               case 'FNAM':
                  break;
               case 'SNAM':
                  if (!subrecord.skip_bytes(sizeof(phase_indices.start)))
                     return;
                  break;
               case 'ENAM':
                  if (!subrecord.skip_bytes(sizeof(phase_indices.end)))
                     return;
                  done = true;
                  break;
               default:
                  return;
            }
            if (done)
               break;
         }
         switch (type) {
            case action_type::dialogue:
               //
               // BGSSceneActionDialogue
               //
               {
                  form_id_t topic;
                  while (auto& subrecord = record.next_subrecord()) {
                     const auto signature = subrecord.signature();
                     if (signature == 'ANAM') {
                        break;
                     } else if (signature == 'DATA') {
                        if (!subrecord.read(topic))
                           return;
                     }
                  }
                  uib.add_outbound_reference(topic);
               }
               break;
            case action_type::package:
               //
               // BGSSceneActionPackage
               //
               {
                  auto sub_uib = uib.spawn_subordinate_on_stack();
                  while (auto& subrecord = record.next_subrecord()) {
                     const auto signature = subrecord.signature();
                     if (signature == 'ANAM') {
                        break;
                     } else if (signature == 'PNAM') {
                        form_id_t id;
                        if (!subrecord.read(id))
                           return;
                        if (id)
                           sub_uib.add_outbound_reference(id);
                     }
                  }
               }
               break;
            case action_type::timer:
               //
               // BGSSceneActionTimer
               //
               while (auto& subrecord = record.next_subrecord()) {
                  const auto signature = subrecord.signature();
                  if (signature == 'ANAM') {
                     break;
                  } else if (signature == 'SNAM') {
                     if (!subrecord.skip_bytes(sizeof(timer_data::duration)))
                        return;
                  }
               }
               break;
            default:
               std::unreachable();
         }
      }

      void Scene::action::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         {
            auto& subrecord = record.open_next_subrecord('ANAM');
            subrecord.write(this->type());
            subrecord.close();
         }
         if (!this->name.empty())
            record.write_string_subrecord('NAM0', this->name);
         {
            auto& subrecord = record.open_next_subrecord('ALID');
            subrecord.write(this->alias_id);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('INAM');
            subrecord.write(this->action_id);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('FNAM');
            subrecord.write(this->flags);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('SNAM');
            subrecord.write(this->phase_indices.start);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('ENAM');
            subrecord.write(this->phase_indices.end);
            subrecord.close();
         }
         if (auto* casted = std::get_if<dialogue_data>(&this->data)) {
            record.write_formID_subrecord('DATA', casted->topic, true);
            {
               auto& subrecord = record.open_next_subrecord('HTID');
               subrecord.write(casted->headtrack_alias_id);
               subrecord.close();
            }
            {
               auto& subrecord = record.open_next_subrecord('DMAX');
               subrecord.write(casted->looping.max);
               subrecord.close();
            }
            {
               auto& subrecord = record.open_next_subrecord('DMIN');
               subrecord.write(casted->looping.min);
               subrecord.close();
            }
            {
               auto& subrecord = record.open_next_subrecord('DEMO');
               subrecord.write(casted->emotion.type);
               subrecord.close();
            }
            {
               auto& subrecord = record.open_next_subrecord('DEVA');
               subrecord.write(casted->emotion.value);
               subrecord.close();
            }
         } else if (auto* casted = std::get_if<package_data>(&this->data)) {
            for (auto& use : casted->packages)
               record.write_formID_subrecord('PNAM', use, true);
         } else if (auto* casted = std::get_if<timer_data>(&this->data)) {
            auto& subrecord = record.open_next_subrecord('SNAM');
            subrecord.write(casted->duration);
            subrecord.close();
         }
         record.open_next_subrecord('ANAM').close();
      }
      void Scene::action::clear(loaded_forms::Form& my_containing_form) noexcept {
         if (auto* casted = std::get_if<dialogue_data>(&this->data)) {
            casted->topic.set(my_containing_form, nullptr);
            casted->emotion = {};
            casted->headtrack_alias_id = -1;
            casted->looping = {};
         } else if (auto* casted = std::get_if<package_data>(&this->data)) {
            clear_form_reference_list(casted->packages, my_containing_form);
         } else if (auto* casted = std::get_if<timer_data>(&this->data)) {
            ;
         }
         this->name.clear();
         this->alias_id = -1;
         this->flags = 0;
      }
      void Scene::action::clone_from(const action& src, loaded_forms::Form& my_containing_form) noexcept {
         this->clear(my_containing_form);

         if (const auto* casted = std::get_if<dialogue_data>(&src.data)) {
            auto& dst = this->data.emplace<dialogue_data>();
            dst.emotion = casted->emotion;
            dst.headtrack_alias_id = casted->headtrack_alias_id;
            dst.looping = casted->looping;
            dst.topic.set(my_containing_form, casted->topic);
         } else if (const auto* casted = std::get_if<package_data>(&src.data)) {
            auto& dst = this->data.emplace<package_data>();
            copy_form_reference_list(my_containing_form, dst.packages, casted->packages);
         } else if (const auto* casted = std::get_if<timer_data>(&src.data)) {
            auto& dst = this->data.emplace<timer_data>();
            dst = *casted;
         }
         this->name = src.name;
         this->alias_id = src.alias_id;
         this->action_id = src.action_id;
         this->flags = src.flags;
         this->phase_indices = src.phase_indices;
      }
      void Scene::action::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
         if (auto* casted = std::get_if<dialogue_data>(&this->data)) {
            casted->topic.clear_if(my_containing_form, target);
         } else if (auto* casted = std::get_if<package_data>(&this->data)) {
            remove_form_from_reference_list(casted->packages, target, my_containing_form);
         } else if (auto* casted = std::get_if<timer_data>(&this->data)) {
            ;
         }
      }
   #pragma endregion

   void Scene::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      size_t destination_actor = 0;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'FNAM':
               subrecord.read(this->scene_flags);
               break;
            case 'PNAM':
               if (auto& dst = this->owning_quest; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::quest, subrecord.signature());
               break;
            case 'INAM':
               subrecord.read(this->last_action_id);
               break;
            case 'CTDA':
               this->loop_conditions.read_next(record, intfc);
               break;
            #pragma region Phases (HNAM+...+HNAM)
               //
               // All phase-related subrecords are handled by a separate loader, triggered 
               // upon opening of a valid HNAM subrecord.
               //
               case 'HNAM':
                  {
                     size_t i      = this->phases.size();
                     auto   result = this->phases.emplace_back().load(record, intfc, i);
                     if (!result) {
                        specific_load_warnings::unterminated_scene_phase notice(
                           intfc.target_stub,
                           i
                        );
                        intfc.log_load_warning(notice);
                        //
                        this->phases.erase(this->phases.end() - 1);
                     }
                  }
                  break;
            #pragma endregion
            #pragma region Actors (ALID, LNAM, DNAM)
               case 'ALID':
                  {
                     uint32_t alias_id;
                     if (subrecord.read(alias_id)) {
                        this->actors.emplace_back().alias_id = alias_id;
                     }
                  }
                  break;
               case 'LNAM':
                  if (destination_actor >= this->actors.size()) {
                     specific_load_warnings::scene_actor_subrecords_out_of_order notice(
                        intfc.target_stub,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                  } else {
                     subrecord.read(this->actors.back().participation_flags);
                  }
                  break;
               case 'DNAM':
                  if (destination_actor >= this->actors.size()) {
                     specific_load_warnings::scene_actor_subrecords_out_of_order notice(
                        intfc.target_stub,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                  } else {
                     subrecord.read(this->actors.back().behavior_flags);
                  }
                  ++destination_actor;
                  break;
            #pragma endregion
            #pragma region Actions (ANAM+...+ANAM)
               //
               // All action-related subrecords are handled by a separate loader, triggered 
               // upon opening of a valid ANAM subrecord.
               //
               case 'ANAM':
                  {
                     action_type type;
                     if (subrecord.read(type)) {
                        auto& dst = this->actions.emplace_back();
                        if (!dst.load(record, intfc, type)) {
                           this->actions.erase(this->actions.end() - 1);
                        }
                     }
                  }
                  break;
            #pragma endregion
            case 'VNAM':
               for (auto& item : this->vnam)
                  if (!subrecord.read(item))
                     break;
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Scene::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t owning_quest;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'PNAM':
               subrecord.read(owning_quest);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ANAM':
               {
                  action_type type;
                  if (subrecord.read(type)) {
                     bool valid = false;
                     switch (type) {
                        case action_type::dialogue:
                        case action_type::package:
                        case action_type::timer:
                           valid = true;
                           break;
                     }
                     if (!valid)
                        break;
                     action::generate_use_info(record, uib, type);
                  }
               }
               break;
            case 'HNAM':
               phase::generate_use_info(record, uib);
               break;
         }
      }
      uib.add_outbound_reference(owning_quest, use_info_entry::flag::dialogue_quest);
   }
   void Scene::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Scene*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->scene_flags = this->scene_flags;
      {
         auto& dst = copy->actions;
         if (!dst.empty()) {
            for (auto& item : dst)
               item.clear(*copy);
            dst.clear();
         }
         const size_t size = this->actions.size();
         dst.resize(size);
         for (size_t i = 0; i < size; ++i)
            dst[i].clone_from(this->actions[i], *copy);
      }
      copy->actors = this->actors;
      {
         auto& dst = copy->phases;
         if (!dst.empty()) {
            for (auto& item : dst)
               item.clear(*copy);
            dst.clear();
         }
         const size_t size = this->phases.size();
         dst.resize(size);
         for (size_t i = 0; i < size; ++i)
            dst[i].clone_from(this->phases[i], *copy);
      }
      copy->owning_quest.set(*copy, this->owning_quest);
      copy->last_action_id = this->last_action_id;
      {
         assert(copy->loop_conditions.empty());
         copy->loop_conditions.append_all_of(*copy, this->loop_conditions);
      }
      copy->vnam = this->vnam;
   }
   void Scene::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('FNAM');
         subrecord.write(this->scene_flags);
         subrecord.close();
      }
      for (auto& item : this->phases)
         item.save(record, intfc);
      for (auto& item : this->actors) {
         {
            auto& subrecord = record.open_next_subrecord('ALID');
            subrecord.write(item.alias_id);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('LNAM');
            subrecord.write(item.participation_flags);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord('DNAM');
            subrecord.write(item.behavior_flags);
            subrecord.close();
         }
      }
      for (auto& item : this->actions)
         item.save(record, intfc);
      //
      // TODO: legacy script
      //
      record.open_next_subrecord('NEXT').close();
      //
      // TODO: legacy script
      //
      record.write_formID_subrecord('PNAM', this->owning_quest);
      {
         auto& subrecord = record.open_next_subrecord('INAM');
         subrecord.write(this->last_action_id);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('VNAM');
         for (auto& item : this->vnam)
            subrecord.write(item);
         subrecord.close();
      }
      for (auto& cnd : this->loop_conditions)
         cnd.save(record, intfc);
   }
   void Scene::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->scene_flags = 0;
      {
         auto& dst = this->actions;
         if (!dst.empty()) {
            for (auto& item : dst)
               item.clear(*this);
            dst.clear();
         }
      }
      this->actors.clear();
      {
         auto& dst = this->phases;
         if (!dst.empty()) {
            for (auto& item : dst)
               item.clear(*this);
            dst.clear();
         }
      }
      this->owning_quest.set(*this, nullptr);
      this->last_action_id = 0;
      this->vnam = { 3, 3, 3, 3 };
      this->loop_conditions.clear(*this);
   }
   void Scene::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& item : this->actions)
         item.sever_outbound_references_to(other, *this);
      for (auto& item : this->phases)
         item.sever_outbound_references_to(other, *this);
      this->owning_quest.clear_if(*this, other);
      for (auto& cnd : this->loop_conditions)
         cnd.sever_outbound_references_to(other, *this);
   }
}