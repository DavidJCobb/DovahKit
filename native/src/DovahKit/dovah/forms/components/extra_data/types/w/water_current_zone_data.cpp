#include "./water_current_zone_data.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/water_current_zone_data_swallowed_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::extra_data;
   }
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result water_current_zone_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      switch (subrecord.signature()) {
         case signature_vel_linear:
            subrecord.read(this->velocity.linear.x);
            subrecord.read(this->velocity.linear.y);
            subrecord.read(this->velocity.linear.z);
            break;
         case signature_vel_rotational:
            subrecord.read(this->velocity.angular.x);
            subrecord.read(this->velocity.angular.y);
            subrecord.read(this->velocity.angular.z);
            break;
         case signature_zone_cell:
         case signature_zone_ref:
            return subrecord_load_result::requires_record;
         default:
            return subrecord_load_result::unrecognized;
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result water_current_zone_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      std::optional<form_reference_t> ref;
      {
         auto& subrecord = record.get_current_subrecord();
         switch (subrecord.signature()) {
            case signature_zone_cell:
               subrecord.read(this->cell);
               intfc.warn_if_ref_is_wrong_type(this->cell, form_type::cell, subrecord);
               break;
            case signature_zone_ref:
               subrecord.read(ref.emplace());
               intfc.warn_if_ref_is_wrong_type(ref.value(), form_type::reference, subrecord);
               break;
            default:
               assert(false && "Wait, what? How did we get here?");
         }
      }

      auto& subrecord = record.next_subrecord();
      if (subrecord.signature() != signature_zone_action) {
         specific_load_warnings::water_current_zone_data_swallowed_subrecord notice(
            intfc.target_stub,
            subrecord.signature()
         );
         intfc.log_load_warning(notice);
      }
      if (ref.has_value()) { // if we're reading XCZR+XCZA...
         auto& ref_use = ref.value();
         bool  found   = false;
         for (auto& item : this->refs) {
            if (item.ref == ref_use) {
               subrecord.read(item.action);
               found = true;
               break;
            }
         }
         if (!found) {
            auto& item = this->refs.emplace_back();
            item.ref = std::move(ref_use);
            subrecord.read(item.action);
         }
      } else { // ...else we're reading XCZC+XCZA
         //
         // NOTE: The CK doesn't read XCZA if XCZC is None.
         //
         subrecord.read(this->action);
      }

      return record_load_result::complete;
   }
   /*virtual*/ void water_current_zone_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      {
         auto& v = this->velocity.linear;
         if (v.x || v.y || v.z) {
            auto& XCVL = record.open_next_subrecord(signature_vel_linear);
            XCVL.write(v.x);
            XCVL.write(v.y);
            XCVL.write(v.z);
            XCVL.close();
         }
      }
      {
         auto& v = this->velocity.angular;
         if (v.x || v.y || v.z) {
            auto& XCVL = record.open_next_subrecord(signature_vel_rotational);
            XCVL.write(v.x);
            XCVL.write(v.y);
            XCVL.write(v.z);
            XCVL.close();
         }
      }
      if (this->cell) {
         record.write_formID_subrecord(signature_zone_cell, this->cell);
         auto& XCZA = record.open_next_subrecord(signature_zone_action);
         XCZA.write(this->action);
         XCZA.close();
      }
      for (auto& item : this->refs) {
         record.write_formID_subrecord(signature_zone_ref, item.ref);
         auto& XCZA = record.open_next_subrecord(signature_zone_action);
         XCZA.write(item.action);
         XCZA.close();
      }
   }
   
   /*static*/ void water_current_zone_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_zone_cell) {
         subrecord.read(uis.form_ids.by_name.water_current_zone_data.cell);
         record.next_subrecord(); // swallow XCZA
         return;
      }
      if (subrecord.signature() == signature_zone_ref) {
         form_id_t form_id;
         if (subrecord.read(form_id)) {
            uis.water_current_zone_data.refs.insert(form_id);
         }
         record.next_subrecord(); // swallow XCZA
         return;
      }
   }
   /*virtual*/ void water_current_zone_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->cell.set(my_owner, nullptr);

      for (auto& item : this->refs) {
         item.ref.set(my_owner, nullptr);
      }
      this->refs.clear();
   }
   /*virtual*/ void water_current_zone_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->cell.clear_if(my_owner, target);

      bool any_removed = false;
      for (auto& item : this->refs) {
         item.ref.clear_if(my_owner, target);
         if (!item.ref)
            any_removed = true;
      }
      if (any_removed)
         std::erase_if(this->refs, [](const auto& item) { return !item.ref; });
   }
   
   /*virtual*/ extra_data* water_current_zone_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new water_current_zone_data;
      clone->action = this->action;
      clone->cell.set(clone_owner, this->cell);
      clone->velocity.linear  = this->velocity.linear;
      clone->velocity.angular = this->velocity.angular;
      for (auto& src_item : this->refs) {
         auto& dst_item = clone->refs.emplace_back();
         dst_item.ref.set(clone_owner, src_item.ref);
         dst_item.action = src_item.action;
      }
      return clone;
   }
}