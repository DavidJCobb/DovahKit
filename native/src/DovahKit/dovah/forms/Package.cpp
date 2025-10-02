#include "Package.h"
#include "_common_cpp.h"
#include "./structs/typed_package_info/_all.h"

#include "../notices/form_load_warnings/by_form_type/package/legacy_type_unrecognized.h"
#include "../notices/form_load_warnings/by_form_type/package/package_changed_legacy_type_during_load.h"
#include "../notices/form_load_warnings/by_form_type/package/package_with_a_template_cannot_itself_be_a_template.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms {
   void Package::_force_type_during_load(legacy_type t, bool complain_on_change, load_order_interfaces::form_load& intfc) {
      if (this->typed_info) {
         if (this->typed_info->is_of_legacy_type(t))
            return;
         if (complain_on_change) {
            specific_load_warnings::package_changed_legacy_type_during_load notice(
               intfc.target_stub,
               this->type,
               t
            );
            intfc.log_load_warning(notice);
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

         case legacy_type::sandbox:
         case legacy_type::sleep:
         case legacy_type::travel:
         case legacy_type::wander:
            this->typed_info = new structs::typed_package_info::generic::with_location(t);
            break;
         case legacy_type::accompany:
            this->typed_info = new structs::typed_package_info::generic::with_target(t);
            break;
         case legacy_type::flee_non_combat:
         case legacy_type::use_magic: // guessed
            this->typed_info = new structs::typed_package_info::generic::with_maybe_each(t);
            break;
         case legacy_type::find:
         case legacy_type::find_deprecated:
         case legacy_type::guard:
            this->typed_info = new structs::typed_package_info::generic::with_target_and_maybe_location(t);
            break;

         default:
            specific_load_warnings::legacy_type_unrecognized notice(
               intfc.target_stub,
               t
            );
            intfc.log_load_warning(notice);

            this->typed_info = new structs::typed_package_info::custom;

            break;
      }
   }
   void Package::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      std::optional<structs::package_location> legacy_location_1;
      std::optional<structs::package_target>   legacy_target_1;

      //
      // The subrecord-reading loop in this function is a little different from 
      // the usual for form loaders; it has a `already_in_next_subrecord` local 
      // variable.
      // 
      // In general, loaders (both mine and Bethesda's) are structured as if 
      // subrecords are unordered. The individual handlers for subrecords may 
      // not behave that way -- the code to read one subrecord may have behavior 
      // contingent on data read from a previous subrecord -- but the code that 
      // deals with the subrecords *themselves* -- opening them, and checking 
      // which signature we're in -- is just a looped switch-case that doesn't 
      // care about subrecord ordering.
      // 
      // Modern package data (TESCustomPackageData in Bethesda's codebase) is 
      // NOT structured that way. Rather, it expects a very specific subrecord 
      // ordering, and in general, it will consume subrecords as it reads them, 
      // such that once all "custom"-type package data is loaded, we're already 
      // in the next subrecord after that data. Bethesda's code (at least as it 
      // was compiled) handles that with a goto; we handle that with a variable 
      // that we use to skip opening the next subrecord out here, in this loop.
      //

      size_t orphaned_condition_count  = 0;
      bool   already_in_next_subrecord = false;
      auto& subrecord = record.get_current_subrecord();
      while ((already_in_next_subrecord && record.get_current_subrecord()) || record.next_subrecord()) {
         already_in_next_subrecord = false;

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
               {
                  legacy_type new_type = (legacy_type)0;
                  subrecord.read(this->general_flags);
                  subrecord.read(new_type);
                  subrecord.read(this->interrupt_override);
                  subrecord.read(this->preferred_speed);
                  subrecord.skip_bytes(1);
                  subrecord.read(this->interrupt_flags);
                  subrecord.read(this->legacy_typed_flags);

                  if (this->typed_info && this->typed_info->is_of_legacy_type(this->type)) {
                     break;
                  }
                  this->_force_type_during_load(new_type, true, intfc);
                  if ((size_t)this->type < packages::all_legacy_type_info.size()) {
                     const auto& info = packages::all_legacy_type_info[(size_t)this->type];
                     if (info.has_location[0] == packages::legacy_type_info::have::no) {
                        legacy_location_1.reset();
                     }
                     if (info.has_target[0] == packages::legacy_type_info::have::no) {
                        legacy_target_1.reset();
                     }
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

            #pragma region Typed package info
            case structs::typed_package_info::ambush::header_subrecord:
               this->_force_type_during_load(legacy_type::ambush, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case 'PKCU':
               this->_force_type_during_load(legacy_type::custom, true, intfc);
               this->typed_info->load(record, intfc);
               //
               // When we finish loading `custom` typed info, we're already in the next subrecord. 
               // The logic for loading TESCustomPackageData just kinda shakes out that way.
               //
               already_in_next_subrecord = true;
               break;
            case structs::typed_package_info::dialogue::header_subrecord:
               this->_force_type_during_load(legacy_type::dialogue, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::escort::header_subrecord:
               this->_force_type_during_load(legacy_type::escort, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::eat::header_subrecord:
               this->_force_type_during_load(legacy_type::eat, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::follow::header_subrecord:
               this->_force_type_during_load(legacy_type::follow, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::patrol::header_subrecord:
               this->_force_type_during_load(legacy_type::patrol, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::use_weapon::header_subrecord:
               this->_force_type_during_load(legacy_type::use_weapon, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            case structs::typed_package_info::use_item_at::header_subrecord:
               this->_force_type_during_load(legacy_type::use_item_at, true, intfc);
               this->typed_info->load(record, intfc);
               break;
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (this->type == packages::legacy_type::custom_template) {
         if (auto* casted = dynamic_cast<structs::typed_package_info::custom*>(this->typed_info)) {
            if (casted->template_package) {
               this->type = packages::legacy_type::custom;
               specific_load_warnings::package_with_a_template_cannot_itself_be_a_template notice(intfc.target_stub);
               intfc.log_load_warning(notice);
            }
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
      
      //
      // Re: `already_in_next_subrecord`: see comments in `Package::load`.
      //
      bool  already_in_next_subrecord = false;
      auto& subrecord = record.get_current_subrecord();
      while ((already_in_next_subrecord && record.get_current_subrecord()) || record.next_subrecord()) {
         already_in_next_subrecord = false;

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
               already_in_next_subrecord = true;
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

   void Package::convert_to_modern(bool use_existing_templates) {
      if (!this->typed_info)
         return;
      if (dynamic_cast<structs::typed_package_info::custom*>(this->typed_info))
         return;

      auto  custom_ptr = std::make_unique<structs::typed_package_info::custom>();
      auto& custom     = *custom_ptr;

      /*//

      //
      // SCRAPPED DUE TO SHEER COMPLEXITY AND DUE TO SOME LEGACY BEHAVIORS 
      // BEING IMPOSSIBLE TO RECONSTRUCT IN MODERN PACKAGES
      //

      //
      // Needed includes if we finish and include this:
      //    "dovah/data/conditions/all_function_info.h"
      //    "./structs/custom_packages/package_data/_all.h"
      //    "./components/conditions/working_condition.h"
      //

      auto _make_packdata_decls_for_procedure = [&custom](dovah::packages::procedure_type type) {
         const auto& info = dovah::packages::all_procedure_type_info[(size_t)type];
         for (size_t i = 0; i < info.param_count; ++i) {
            const auto& param = info.params[i];
            auto& decl = custom.data.declarations.entries.emplace_back();
            decl.unique_id = i;
            decl.name      = param.name;
            decl.is_public = true;
         }
      };

      #pragma region Helpers to make packdata
         auto _make_bool_packdata = [&custom](
            uint8_t uid,
            std::string_view name,
            bool  value     = false,
            bool  is_public = true
         ) {
            using packdata_decl  = structs::custom_packages::package_data_declaration_map::entry;
            using packdata_defn  = structs::custom_packages::package_data_value_map::entry;
            using packdata_value = structs::custom_packages::package_data_bool;
            //
            custom.data.declarations.entries.emplace_back(packdata_decl{
               .unique_id = uid,
               .name      = std::move(std::string(name)),
               .is_public = is_public,
            });
            auto& pd_value = custom.data.values.entries.emplace_back();
            pd_value.unique_id = uid;
            pd_value.value     = std::make_unique<packdata_value>();
            ((packdata_value*)pd_value.value.get())->value = value;
         };
         auto _make_float_packdata = [&custom](
            uint8_t uid,
            std::string_view name,
            float value     = 0.0F,
            bool  is_public = true
         ) {
            using packdata_decl  = structs::custom_packages::package_data_declaration_map::entry;
            using packdata_value = structs::custom_packages::package_data_float;
            //
            custom.data.declarations.entries.emplace_back(packdata_decl{
               .unique_id = uid,
               .name      = std::move(std::string(name)),
               .is_public = is_public,
            });
            auto& pd_value = custom.data.values.entries.emplace_back();
            pd_value.unique_id = uid;
            pd_value.value     = std::make_unique<packdata_value>();
            ((packdata_value*)pd_value.value.get())->value = value;
         };
         auto _make_object_list_packdata = [this, &custom](
            uint8_t uid,
            std::string_view name,
            bool  is_public = true
         ) {
            using packdata_decl  = structs::custom_packages::package_data_declaration_map::entry;
            using packdata_value = structs::custom_packages::package_data_object_list;
            //
            custom.data.declarations.entries.emplace_back(packdata_decl{
               .unique_id = uid,
               .name      = std::move(std::string(name)),
               .is_public = is_public,
            });
            auto& pd_value = custom.data.values.entries.emplace_back();
            pd_value.unique_id = uid;
            pd_value.value     = std::make_unique<packdata_value>();
         };
         auto _make_location_packdata = [this, &custom](
            uint8_t uid,
            std::string_view name,
            dovah::packages::location_type value,
            bool  is_public = true
         ) {
            using packdata_decl  = structs::custom_packages::package_data_declaration_map::entry;
            using packdata_value = structs::custom_packages::package_data_location;
            //
            custom.data.declarations.entries.emplace_back(packdata_decl{
               .unique_id = uid,
               .name      = std::move(std::string(name)),
               .is_public = is_public,
            });
            auto& pd_value = custom.data.values.entries.emplace_back();
            pd_value.unique_id = uid;
            pd_value.value     = std::make_unique<packdata_value>();
            ((packdata_value*)pd_value.value.get())->data.set_type(*this, value);
         };
         auto _make_target_selector_packdata = [this, &custom](
            uint8_t uid,
            std::string_view name,
            bool  is_public = true
         ) {
            using packdata_decl  = structs::custom_packages::package_data_declaration_map::entry;
            using packdata_value = structs::custom_packages::package_data_target_selector;
            //
            custom.data.declarations.entries.emplace_back(packdata_decl{
               .unique_id = uid,
               .name      = std::move(std::string(name)),
               .is_public = is_public,
            });
            auto& pd_value = custom.data.values.entries.emplace_back();
            pd_value.unique_id = uid;
            pd_value.value     = std::make_unique<packdata_value>();
         };
      #pragma endregion

      using procedure_node = structs::custom_packages::procedure_node;
      using procedure_data = structs::custom_packages::procedure_node_data::procedure;
      using procedure_type = dovah::packages::procedure_type;
      using branch_data    = structs::custom_packages::procedure_node_data::branch;
      using branch_type    = dovah::packages::procedure_tree_branch_type;

      auto _make_root_branch = [&custom](branch_type type) -> branch_data& {
         auto& root_ptr  = custom.procedures.root = std::make_unique<procedure_node>();
         auto& root_data = root_ptr->data.emplace<branch_data>();
         root_data.branch_type = type;
         return root_data;
      };
      auto _make_procedure_node = [](branch_data& parent_branch, procedure_type type) -> procedure_node& {
         std::unique_ptr node_ptr = std::make_unique<procedure_node>();
         auto* node = node_ptr.get();
         parent_branch.children.push_back(std::move(node_ptr));

         auto& node_data = node->data.emplace<procedure_data>();
         node_data.type = type;

         return *node;
      };

      if (dynamic_cast<structs::typed_package_info::ambush*>(this->typed_info)) {
         auto& root_data          = _make_root_branch(branch_type::simultaneous);
         auto& wander_normal_node = _make_procedure_node(root_data, procedure_type::wander);
         auto& wander_sneak_node  = _make_procedure_node(root_data, procedure_type::wander);
         auto& guard_node         = _make_procedure_node(root_data, procedure_type::guard);
         {
            auto& node_data = std::get<procedure_data>(wander_sneak_node.data);
            auto& flag_over = node_data.flag_overrides.emplace();
            flag_over.general.set |= general_flag::always_sneak;
         }

         enum class packdata_ids : uint8_t {
            wait_location,
            wait_min_distance,
            wait_preferred_path,
            wait_ride_horse,
            hide_while_ambushing,
            ambush_location,
            ambush_target,
            always_zero_float,
         };
         //
         // BUG: We aren't preserving the values from the original Ambush package!
         //
         _make_location_packdata((int)packdata_ids::wait_location, "Wait Location", dovah::packages::location_type::near_editor_location, true);
         _make_bool_packdata((int)packdata_ids::wait_preferred_path, "Waiting - Prefer Preferred Paths", false, true);
         _make_bool_packdata((int)packdata_ids::wait_ride_horse, "Waiting - Ride Horse if Possible", false, true);
         _make_float_packdata((int)packdata_ids::wait_min_distance, "Waiting - Min Distance", 0.0F, false);
         _make_bool_packdata((int)packdata_ids::hide_while_ambushing, "Hide While Ambushing", false, true);
         _make_location_packdata((int)packdata_ids::ambush_location, "Ambush Location", dovah::packages::location_type::near_editor_location, true);
         _make_target_selector_packdata((int)packdata_ids::ambush_target, "Ambush Target", true);
         _make_float_packdata((int)packdata_ids::always_zero_float, "Zero Float", 0.0F, false);

         {
            auto& data = std::get<procedure_data>(wander_normal_node.data);
            data.parameter_unique_ids = {
               (uint8_t)packdata_ids::wait_location,
               (uint8_t)packdata_ids::wait_preferred_path,
               (uint8_t)packdata_ids::wait_ride_horse,
               (uint8_t)packdata_ids::wait_min_distance,
            };

            components::conditions::working_condition hide;
            hide.set_function_id(dovah::conditions::function_id_by_name("GetNumericPackageData"));
            hide.parameters[0].emplace<uint32_t>((int)packdata_ids::hide_while_ambushing);
            hide.comparison.op = components::conditions::comparison_operator::equal;
            hide.comparison.operand.emplace<float>(0);
            //
            wander_normal_node.conditions.append(*this, hide);
         }
         {
            auto& data = std::get<procedure_data>(wander_sneak_node.data);
            data.parameter_unique_ids = {
               (uint8_t)packdata_ids::wait_location,
               (uint8_t)packdata_ids::wait_preferred_path,
               (uint8_t)packdata_ids::wait_ride_horse,
               (uint8_t)packdata_ids::wait_min_distance,
            };

            components::conditions::working_condition hide;
            hide.set_function_id(dovah::conditions::function_id_by_name("GetNumericPackageData"));
            hide.parameters[0].emplace<uint32_t>((int)packdata_ids::hide_while_ambushing);
            hide.comparison.op = components::conditions::comparison_operator::not_equal;
            hide.comparison.operand.emplace<float>(0);
            //
            wander_sneak_node.conditions.append(*this, hide);
         }
         std::get<procedure_data>(guard_node.data).parameter_unique_ids = {
            (uint8_t)packdata_ids::ambush_location,
            (uint8_t)packdata_ids::ambush_target,
            (uint8_t)packdata_ids::always_zero_float,
            (uint8_t)packdata_ids::always_zero_float,
         };
      } else if (dynamic_cast<structs::typed_package_info::dialogue*>(this->typed_info)) {
         //
         // This isn't even possible to fully recreate using just one modern package. 
         // Legacy Dialogue packages applied to a subject can override the AI behavior 
         // of the target, similar to what Escort procedures do, but the particular 
         // override in question cannot be (accurately) recreated using Escort.
         //
         // The modern replacement for those cases would be packages applied to both 
         // actors in tandem, but the main use case (making two actors stop to talk to 
         // each other) would generally just be a Scene nowadays.
         //
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::eat*>(this->typed_info)) {
         enum class packdata_ids : uint8_t {
            food_criteria,
            allow_search,
            search_location,
            eat_location,
            always_false_bool,
         };
         _make_bool_packdata((int)packdata_ids::allow_search, "False", true, true);
         _make_location_packdata((int)packdata_ids::search_location, "Food Search Location", dovah::packages::location_type::near_editor_location, true);
         _make_location_packdata((int)packdata_ids::eat_location, "Eat Location", dovah::packages::location_type::near_editor_location, true);
         _make_bool_packdata((int)packdata_ids::always_false_bool, "False", false, false);
         static_assert(false, "TODO: remaining packdata (food criteria)");

         auto& root_data = _make_root_branch(branch_type::sequence);


         static_assert(false, "TODO: procedure tree");
         //
         // based on Bethesda's modern Eat template we'd want something like this:
         //
         // - Sequence // repeat when complete
         //    - Simultaneous // conditional on if "Allow Search" is enabled
         //       - Find // food
         //       - Sandbox
         //    - Acquire // conditional on if "Allow Search" is enabled
         //    - Sequence
         //       - Simultaneous
         //          - Find // chair
         //          - Sandbox
         //       - Sit
         //    - Simultaneous
         //       - Eat
         //       - Sit

      } else if (dynamic_cast<structs::typed_package_info::escort*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::follow*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::patrol*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::use_item_at*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::use_weapon*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::generic::with_location*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::generic::with_maybe_each*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::generic::with_target*>(this->typed_info)) {
         static_assert(false, "TODO");
      } else if (dynamic_cast<structs::typed_package_info::generic::with_target_and_maybe_location*>(this->typed_info)) {
         static_assert(false, "TODO");
      }
      //*/
      if (this->typed_info) {
         this->typed_info->clear(*this);
         delete this->typed_info;
      }
      this->typed_info = custom_ptr.release();
   }

   #pragma region Record skimmers
      #pragma region legacy_type
         void Package::record_skimmers::legacy_type::skim_subrecord(tes_subrecord_reader& subrecord) {
            using result_type = packages::legacy_type;

            switch (subrecord.signature()) {
               case 'PKDT':
                  subrecord.skip_bytes(sizeof(Package::general_flags));
                  {
                     result_type prior = this->result.value_or(result_type::invalid);
                     result_type after = result_type::invalid;
                     subrecord.read(after);
                     this->result = after;
                     if (prior != after && this->custom_has_template)
                        this->custom_has_template = false;
                  }
                  break;
               #pragma region Typed package info
               case structs::typed_package_info::ambush::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::ambush;
                  break;
               case 'PKCU':
                  if (this->result != result_type::custom && this->result != result_type::custom_template) {
                     this->result = result_type::custom;
                  }
                  {
                     form_id_t template_package;
                     subrecord.skip_bytes(sizeof(uint32_t)); // package data count
                     subrecord.read(template_package);

                     this->custom_has_template = !!template_package;
                  }
                  break;
               case structs::typed_package_info::dialogue::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::dialogue;
                  break;
               case structs::typed_package_info::escort::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::escort;
                  break;
               case structs::typed_package_info::eat::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::eat;
                  break;
               case structs::typed_package_info::follow::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::follow;
                  break;
               case structs::typed_package_info::patrol::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::patrol;
                  break;
               case structs::typed_package_info::use_weapon::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::use_weapon;
                  break;
               case structs::typed_package_info::use_item_at::header_subrecord:
                  this->custom_has_template = false;
                  this->result = result_type::use_item_at;
                  break;
               #pragma endregion
            }
         }
         void Package::record_skimmers::legacy_type::finalize() {
            using result_type = packages::legacy_type;
            if (this->custom_has_template && this->result == result_type::custom_template) {
               this->result = result_type::custom;
            }
         }
      #pragma endregion
   #pragma endregion
}