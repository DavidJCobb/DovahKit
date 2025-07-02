#include "./ambush.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void ambush::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
   }
   /*virtual*/ void ambush::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      record.open_next_subrecord(header_subrecord).close();
      {
         auto& loc = this->ambush_location;
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* ambush::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<ambush>();
      auto* clone     = clone_ptr.get();

      clone->trigger_location.clone_from(this->trigger_location, owner_of_clone);
      clone->ambush_location.clone_from(this->ambush_location, owner_of_clone);
      {
         auto& src = this->ambush_target;
         auto& dst = clone->ambush_target;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }

      return clone_ptr.release();
   }
   /*virtual*/ void ambush::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->trigger_location.sever_outbound_references_to(other, my_owner);
      this->ambush_location.sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->ambush_target; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void ambush::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->trigger_location.clear(my_owner);
      this->ambush_location.clear(my_owner);
      if (auto& opt = this->ambush_target; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
   }
}