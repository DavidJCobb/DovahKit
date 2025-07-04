#include "./procedure.h"
#include "../../../_common_cpp.h"

#include "../../../../notices/form_load_warnings/by_form_type/package/procedure_has_extra_parameters.h"
#include "../../../../notices/form_load_warnings/by_form_type/package/procedure_missing_required_parameter.h"
#include "../../../../notices/form_load_warnings/by_form_type/package/procedure_typename_unrecognized.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace {
   constexpr const auto typename_mapping = []() {
      using type = dovah::packages::procedure_type;
      using pair = std::pair<std::string_view, type>;
      return std::array{
         pair{ "Acquire", type::acquire },
         pair{ "Activate", type::activate },
         pair{ "DialogueActivate", type::dialogue_activate },
         pair{ "Dialogue", type::dialogue },
         pair{ "Done", type::done },
         pair{ "Eat", type::eat },
         pair{ "Escort", type::escort },
         pair{ "Find", type::find },
         pair{ "Flee", type::flee },
         pair{ "FlightGrab", type::flight_grab },
         pair{ "Follow", type::follow },
         pair{ "FollowTo", type::follow_to },
         pair{ "ForceGreet", type::force_greet },
         pair{ "Guard", type::guard },
         pair{ "HoldPosition", type::hold_position },
         pair{ "Hover", type::hover },
         pair{ "KeepAnEyeOn", type::keep_an_eye_on },
         pair{ "LockDoors", type::lock_doors },
         pair{ "Orbit", type::orbit },
         pair{ "Patrol", type::patrol },
         pair{ "Pursue", type::pursue },
         pair{ "Sandbox", type::sandbox },
         pair{ "Say", type::say },
         pair{ "Shout", type::shout },
         pair{ "Sit", type::sit },
         pair{ "Sleep", type::sleep },
         pair{ "Travel", type::travel },
         pair{ "UnlockDoors", type::unlock_doors },
         pair{ "UseIdleMarker", type::use_idle_marker },
         pair{ "UseMagic", type::use_magic },
         pair{ "UseWeapon", type::use_weapon },
         pair{ "Wait", type::wait },
         pair{ "Wander", type::wander },
      };
   }();
}

namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data {
   void procedure::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_procedure_type) {
            std::string serialized_typename;
            subrecord.read(serialized_typename);

            bool found = false;
            for (auto& item : typename_mapping) {
               if (item.first == serialized_typename) {
                  found = true;
                  this->type = item.second;
                  break;
               }
            }
            if (!found) {
               specific_load_warnings::procedure_typename_unrecognized notice(
                  intfc.target_stub,
                  serialized_typename
               );
               intfc.log_load_warning(notice);
            }

            record.next_subrecord();
         }
      }
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_flags) {
            subrecord.read(this->flags);
            record.next_subrecord();
         }
      }
      //
      // Load parameter IDs:
      //
      {
         size_t param_count = 0;
         if (this->type != procedure_type::invalid && (size_t)this->type < packages::all_procedure_type_info.size()) {
            param_count = packages::all_procedure_type_info[(size_t)this->type].param_count;
         }
         this->parameter_unique_ids.resize(param_count);
         for (size_t i = 0; i < param_count; ++i) {
            auto& subrecord = record.get_current_subrecord();
            switch (subrecord.signature()) {
               case subrecord_param_id_legacy:
                  {
                     uint32_t param = 0;
                     subrecord.read(param);
                     record.next_subrecord();
                     this->parameter_unique_ids[i] = (uint8_t)param;
                  }
                  break;
               case subrecord_param_id_modern:
                  {
                     uint8_t param = 0;
                     subrecord.read(param);
                     record.next_subrecord();
                     this->parameter_unique_ids[i] = param;
                  }
                  break;
               default:
                  this->parameter_unique_ids[i] = 0xFF; // none
                  //
                  // The CK *does not* advance to the next subrecord here, meaning that if the list 
                  // ends early, we just set all remaining parameter IDs to "none" and then leave 
                  // the unexpected subrecord for whatever will read stuff next.
                  //
                  break;
            }
         }

         // Warn on extra parameters
         {
            size_t extra = 0;
            while (true) {
               bool is_extra = false;
               switch (record.get_current_subrecord().signature()) {
                  case subrecord_param_id_legacy:
                  case subrecord_param_id_modern:
                     is_extra = true;
                     break;
               }
               if (!is_extra)
                  break;

               record.next_subrecord();
               ++extra;
            }
            if (extra > 0) {
               specific_load_warnings::procedure_has_extra_parameters notice(
                  intfc.target_stub,
                  extra
               );
               intfc.log_load_warning(notice);
            }
         }
      }
      //
      // Validate parameter IDs:
      //
      if (this->type != procedure_type::invalid && (size_t)this->type < packages::all_procedure_type_info.size()) {
         const auto& info = packages::all_procedure_type_info[(size_t)this->type];
         for (size_t i = 0; i < info.param_count; ++i) {
            auto  id     = this->parameter_unique_ids[i];
            auto& p_info = info.params[i];
            if (id == 0xFF && p_info.required) {
               specific_load_warnings::procedure_missing_required_parameter notice(
                  intfc.target_stub,
                  i,
                  std::string(p_info.name)
               );
               intfc.log_load_warning(notice);
               break;
            }
         }
      }
      //
      // Flag overrides:
      //
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == package_flag_overrides::subrecord_modern) {
            auto& opt = this->flag_overrides;
            if (!opt.has_value())
               opt.emplace();
            opt.value().load(subrecord, intfc);
            record.next_subrecord();
         }
      }
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == package_flag_overrides::subrecord_legacy) {
            auto& opt = this->flag_overrides;
            if (!opt.has_value())
               opt.emplace();
            opt.value().load(subrecord, intfc);
            record.next_subrecord();
         }
      }
   }
   /*static*/ void procedure::generate_use_info(tes_record_reader& record, form_stub_use_info_builder&) {
      //
      // We don't actually contain any uses, but we need to skip the right number 
      // of subrecords.
      //
      if (record.get_current_subrecord().signature() == subrecord_procedure_type)
         record.next_subrecord();
      if (record.get_current_subrecord().signature() == subrecord_flags)
         record.next_subrecord();
      //
      // Parameter IDs:
      //
      while (true) {
         bool is_extra = false;
         switch (record.get_current_subrecord().signature()) {
            case subrecord_param_id_legacy:
            case subrecord_param_id_modern:
               is_extra = true;
               break;
         }
         if (!is_extra)
            break;

         record.next_subrecord();
      }
      //
      // Flag overrides:
      //
      if (record.get_current_subrecord().signature() == package_flag_overrides::subrecord_modern)
         record.next_subrecord();
      if (record.get_current_subrecord().signature() == package_flag_overrides::subrecord_legacy)
         record.next_subrecord();
   }
   void procedure::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      {
         std::string_view name = "";
         for (auto& item : typename_mapping) {
            if (item.second == this->type) {
               name = item.first;
               break;
            }
         }
         record.write_string_subrecord(subrecord_procedure_type, name.data());
      }
      {
         auto& subrecord = record.open_next_subrecord(subrecord_flags);
         subrecord.write(this->flags);
         subrecord.close();
      }
      for (auto id : this->parameter_unique_ids) {
         auto& subrecord = record.open_next_subrecord(subrecord_param_id_modern);
         subrecord.write(id);
         subrecord.close();
      }
      if (auto& opt = this->flag_overrides; opt.has_value()) {
         auto& subrecord = record.open_next_subrecord(package_flag_overrides::subrecord_modern);
         opt.value().save(subrecord, intfc);
         subrecord.close();
      }
   }
}