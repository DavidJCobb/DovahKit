#include "water_current_zone_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result water_current_zone_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
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
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void water_current_zone_data::save(tes_record_writer& record) {
      auto& XCVL = record.open_next_subrecord(signature_vel_linear);
      XCVL.write(this->velocity.linear.x);
      XCVL.write(this->velocity.linear.y);
      XCVL.write(this->velocity.linear.z);
      XCVL.close();
      auto& XCVR = record.open_next_subrecord(signature_vel_rotational);
      XCVR.write(this->velocity.angular.x);
      XCVR.write(this->velocity.angular.y);
      XCVR.write(this->velocity.angular.z);
      XCVR.close();
   }
   basic_extra_data* water_current_zone_data::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new water_current_zone_data;
      clone->velocity.linear  = this->velocity.linear;
      clone->velocity.angular = this->velocity.angular;
      return clone;
   }
}