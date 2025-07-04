#include "Package.h"
#include "_common_cpp.h"
#include "./structs/typed_package_info/_all.h"

namespace dovah::loaded_forms {
   void Package::_force_type_during_load(legacy_type t, bool complain_on_change) {
      if (this->typed_info) {
         if (this->typed_info->is_of_legacy_type(t))
            return;
         if (complain_on_change) {
            static_assert(false, "TODO: Warn on type change");
         }
         delete this->typed_info;
         this->typed_info = nullptr;
      }
      this->type = t;
      switch (t) {
         case legacy_type::ambush:
            this->typed_info = new structs::typed_package_info::ambush;
            break;
         case legacy_type::custom:
         case legacy_type::custom_template:
            this->typed_info = new structs::typed_package_info::custom;
            break;
         case legacy_type::dialogue:
            this->typed_info = new structs::typed_package_info::dialogue;
            break;
         case legacy_type::eat:
            this->typed_info = new structs::typed_package_info::eat;
            break;
         case legacy_type::escort:
            this->typed_info = new structs::typed_package_info::escort;
            break;
         case legacy_type::follow:
            this->typed_info = new structs::typed_package_info::follow;
            break;
         case legacy_type::patrol:
            this->typed_info = new structs::typed_package_info::patrol;
            break;
         case legacy_type::use_item_at:
            this->typed_info = new structs::typed_package_info::use_item_at;
            break;
         case legacy_type::use_weapon:
            this->typed_info = new structs::typed_package_info::use_weapon;
            break;

         default:
            static_assert(false, "TODO: generic typed info. Note that some types may or may not have a primary location or primary target.");
            break;
      }
   }
   void Package::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      std::optional<structs::package_location> legacy_location_1;
      std::optional<structs::package_target>   legacy_target_1;

      size_t orphaned_condition_count = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;

            case 'CTDA':
               this->conditions.read_next(record, intfc);
               break;
            case components::idle_collection::subrecord_signature_array:
            case components::idle_collection::subrecord_signature_count:
            case components::idle_collection::subrecord_signature_flags:
            case components::idle_collection::subrecord_signature_timer:
               this->idles.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'PKDT':
               subrecord.read(this->general_flags);
               subrecord.read(this->type);
               subrecord.read(this->interrupt_override);
               subrecord.read(this->preferred_speed);
               subrecord.skip_bytes(1);
               subrecord.read(this->interrupt_flags);
               subrecord.read(this->legacy_typed_flags);

               if (this->typed_info && this->typed_info->is_of_legacy_type(this->type)) {
                  break;
               }
               static_assert(false, "TODO: Warn: Package re-typed itself!");
               this->_force_type_during_load(this->type, false);
               if ((size_t)this->type < packages::all_legacy_type_info.size()) {
                  const auto& info = packages::all_legacy_type_info[(size_t)this->type];
                  if (info.has_location[0] == packages::legacy_type_info::have::no) {
                     legacy_location_1.reset();
                  }
                  if (info.has_target[0] == packages::legacy_type_info::have::no) {
                     legacy_target_1.reset();
                  }
               }
               break;
            case structs::package_schedule::subrecord:
               this->schedule.load(subrecord, intfc);
               break;
            case 'CNAM':
               if (auto& form = this->combat_style; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::combat_style, subrecord.signature());
               break;
            case 'QNAM':
               if (auto& form = this->owning_quest; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::quest, subrecord.signature());
               break;
            case 'POBA':
               this->events.begin.load(record, intfc);
               break;
            case 'POEA':
               this->events.end.load(record, intfc);
               break;
            case 'POCA':
               this->events.change.load(record, intfc);
               break;

            case structs::package_location::subrecord_package_data:
               legacy_location_1.emplace().load(subrecord, intfc, *this);
               break;
            case structs::package_target::subrecord_legacy:
            case structs::package_target::subrecord_modern:
               legacy_target_1.emplace().load(subrecord, intfc, *this);
               break;

            #pragma region Custom package data
            case structs::typed_package_info::ambush::header_subrecord:
               this->_force_type_during_load(legacy_type::ambush, true);
               break;
            case 'PKCU':
               this->_force_type_during_load(legacy_type::custom, true);
               break;
            case structs::typed_package_info::dialogue::header_subrecord:
               this->_force_type_during_load(legacy_type::dialogue, true);
               break;
            case structs::typed_package_info::escort::header_subrecord:
               this->_force_type_during_load(legacy_type::escort, true);
               break;
            case structs::typed_package_info::eat::header_subrecord:
               this->_force_type_during_load(legacy_type::eat, true);
               break;
            case structs::typed_package_info::follow::header_subrecord:
               this->_force_type_during_load(legacy_type::follow, true);
               break;
            case structs::typed_package_info::patrol::header_subrecord:
               this->_force_type_during_load(legacy_type::patrol, true);
               break;
            case structs::typed_package_info::use_weapon::header_subrecord:
               this->_force_type_during_load(legacy_type::use_weapon, true);
               break;
            case structs::typed_package_info::use_item_at::header_subrecord:
               this->_force_type_during_load(legacy_type::use_item_at, true);
               break;
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Package::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      legacy_type last_seen_type = legacy_type::invalid;

      components::idle_collection::use_info_state idles;
      struct {
         structs::package_event_addon::use_info_state begin;
         structs::package_event_addon::use_info_state end;
         structs::package_event_addon::use_info_state change;
      } events;
      structs::package_location::use_info_state legacy_location_1;
      structs::package_target::use_info_state   legacy_target_1;
      form_id_t combat_style;
      form_id_t owning_quest;
      auto typed_uib = uib.spawn_subordinate_on_stack();

      auto _handle_typed_header = [&]<legacy_type NewType>() {
         {
            bool changed = false;
            if constexpr (NewType == legacy_type::custom || NewType == legacy_type::custom_template) {
               changed = last_seen_type != legacy_type::custom && last_seen_type != legacy_type::custom_template;
            } else {
               changed = last_seen_type != NewType;
            }
            if (!changed)
               return;
         }

         typed_uib.clear_pending_use_info();

         constexpr auto& info = packages::all_legacy_type_info[(size_t)NewType];
         if (info.has_location[0] == packages::legacy_type_info::have::no) {
            legacy_location_1 = {};
         }
         if (info.has_target[0] == packages::legacy_type_info::have::no) {
            legacy_target_1 = {};
         }

         last_seen_type = NewType;
      };

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case components::idle_collection::subrecord_signature_array:
            case components::idle_collection::subrecord_signature_count:
            case components::idle_collection::subrecord_signature_flags:
            case components::idle_collection::subrecord_signature_timer:
               idles.read(subrecord);
               break;
            case 'VMAD':
               components::papyrus::attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'PKDT':
               {
                  legacy_type new_type = (legacy_type)0;
                  subrecord.skip_bytes(4);
                  subrecord.read(new_type);
                  if (new_type != last_seen_type) {
                     if (
                        (new_type       == legacy_type::custom || new_type       == legacy_type::custom_template) &&
                        (last_seen_type == legacy_type::custom || last_seen_type == legacy_type::custom_template)
                     ) {
                        break;
                     }
                     //
                     // The type changed.
                     //
                     typed_uib.clear_pending_use_info();
                     if ((size_t)new_type < packages::all_legacy_type_info.size()) {
                        const auto& info = packages::all_legacy_type_info[(size_t)new_type];
                        if (info.has_location[0] == packages::legacy_type_info::have::no) {
                           legacy_location_1 = {};
                        }
                        if (info.has_target[0] == packages::legacy_type_info::have::no) {
                           legacy_target_1 = {};
                        }
                     }
                  }
                  last_seen_type = new_type;
               }
               break;

            case 'CNAM':
               subrecord.read(combat_style);
               break;
            case 'QNAM':
               subrecord.read(owning_quest);
               break;

            case 'POBA':
               events.begin.generate_use_info(record);
               break;
            case 'POEA':
               events.end.generate_use_info(record);
               break;
            case 'POCA':
               events.change.generate_use_info(record);
               break;

            case structs::package_location::subrecord_package_data:
               legacy_location_1.generate_use_info(subrecord);
               break;
            case structs::package_target::subrecord_legacy:
            case structs::package_target::subrecord_modern:
               legacy_target_1.generate_use_info(subrecord);
               break;
               
            #pragma region Custom package data
            case structs::typed_package_info::ambush::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::ambush>();
               structs::typed_package_info::ambush::generate_header_use_info(record, typed_uib);
               break;
            case 'PKCU':
               _handle_typed_header.template operator()<legacy_type::custom>();
               structs::typed_package_info::custom::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::dialogue::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::dialogue>();
               structs::typed_package_info::dialogue::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::escort::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::escort>();
               structs::typed_package_info::escort::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::eat::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::eat>();
               structs::typed_package_info::eat::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::follow::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::follow>();
               structs::typed_package_info::follow::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::patrol::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::patrol>();
               structs::typed_package_info::patrol::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::use_weapon::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::use_weapon>();
               structs::typed_package_info::use_weapon::generate_header_use_info(record, typed_uib);
               break;
            case structs::typed_package_info::use_item_at::header_subrecord:
               _handle_typed_header.template operator()<legacy_type::use_item_at>();
               structs::typed_package_info::use_item_at::generate_header_use_info(record, typed_uib);
               break;
            #pragma endregion
         }
      }
      idles.commit(uib);
      events.begin.commit_to(uib);
      events.end.commit_to(uib);
      events.change.commit_to(uib);
      uib.add_outbound_reference(combat_style);
      uib.add_outbound_reference(owning_quest);

      // typed package info:
      legacy_location_1.commit_to(uib);
      legacy_target_1.commit_to(uib);

      typed_uib.commit();
   }
   void Package::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Package*)out;

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);
      copy->idles.clone_from(this->idles, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->general_flags = this->general_flags;
      copy->interrupt_flags = this->interrupt_flags;
      copy->interrupt_override = this->interrupt_override;
      copy->legacy_typed_flags = this->legacy_typed_flags;
      copy->preferred_speed = this->preferred_speed;
      copy->schedule = this->schedule;

      copy->combat_style.set(*copy, this->combat_style);
      copy->owning_quest.set(*copy, this->owning_quest);

      copy->type = this->type;
      if (auto* v = copy->typed_info) {
         v->clear(*copy);
         copy->typed_info = nullptr;
         delete v;
      }
      if (this->typed_info) {
         copy->typed_info = this->typed_info->clone(*copy);
      }

      for (size_t i = 0; i < this->events.list.size(); ++i) {
         auto& src = this->events.list[i];
         auto& dst = copy->events.list[i];
         dst.clone_from(src, *copy);
      }
   }
   void Package::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('PKDT');
         subrecord.write(this->general_flags);
         subrecord.write(this->type);
         subrecord.write(this->interrupt_override);
         subrecord.write(this->preferred_speed);
         subrecord.skip_bytes(1);
         subrecord.write(this->interrupt_flags);
         subrecord.write(this->legacy_typed_flags);
         subrecord.close();
      }
      if (this->typed_info) {
         if (auto* loc = this->typed_info->get_location_1()) {
            auto& subrecord = record.open_next_subrecord(structs::package_location::subrecord_package_data);
            loc->save(subrecord, intfc);
            subrecord.close();
         }
         if (auto* loc = this->typed_info->get_location_2()) {
            auto& subrecord = record.open_next_subrecord(structs::package_location::subrecord_legacy_second);
            loc->save(subrecord, intfc);
            subrecord.close();
         }
      }
      {
         auto& subrecord = record.open_next_subrecord(structs::package_schedule::subrecord);
         this->schedule.save(subrecord, intfc);
         subrecord.close();
      }
      if (this->typed_info) {
         if (auto* tgt = this->typed_info->get_target_1()) {
            auto& subrecord = record.open_next_subrecord(structs::package_target::subrecord_modern);
            tgt->save(subrecord, intfc);
            subrecord.close();
         }
      }
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      this->idles.save(record, intfc);
      record.write_formID_subrecord('CNAM', this->combat_style, true);
      record.write_formID_subrecord('QNAM', this->owning_quest, true);
      if (this->typed_info) {
         this->typed_info->save(record, intfc);
      }
      {
         record.open_next_subrecord('POBA').close();
         this->events.begin.save(record, intfc);
      }
      {
         record.open_next_subrecord('POEA').close();
         this->events.end.save(record, intfc);
      }
      {
         record.open_next_subrecord('POCA').close();
         this->events.change.save(record, intfc);
      }
   }
   void Package::_clear_impl() noexcept {
      this->conditions.clear(*this);
      this->idles.clear(*this);
      this->script_data.clear(*this);

      this->general_flags = 0;
      this->interrupt_flags = 0;
      this->legacy_typed_flags = 0;
      this->schedule = {};
      this->preferred_speed = preferred_movement_speed::run;

      this->interrupt_override = packages::interrupt_override_type::none;
      this->type = legacy_type::custom;
      if (auto* v = this->typed_info) {
         this->typed_info = nullptr;
         delete v;
      }

      for (auto& event : this->events.list) {
         event.clear(*this);
      }

      this->combat_style.set(*this, nullptr);
      this->owning_quest.set(*this, nullptr);
   }
   void Package::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);
      this->idles.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      if (auto* v = this->typed_info) {
         v->sever_outbound_references_to(other, *this);
      }
      for (auto& event : this->events.list) {
         event.sever_outbound_references_to(other, *this);
      }

      this->combat_style.clear_if(*this, other);
      this->owning_quest.clear_if(*this, other);
   }
}