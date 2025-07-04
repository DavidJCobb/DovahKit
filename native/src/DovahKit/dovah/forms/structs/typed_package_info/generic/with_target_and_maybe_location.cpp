#include "./with_target_and_maybe_location.h"
#include <memory>
#include "../../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info::generic {
   /*virtual*/ void with_target_and_maybe_location::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
   }
   /*virtual*/ void with_target_and_maybe_location::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
   }
   /*virtual*/ base* with_target_and_maybe_location::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<with_target_and_maybe_location>();
      auto* clone     = clone_ptr.get();
      
      {
         auto& src = this->location;
         auto& dst = clone->location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->target.clone_from(this->target, owner_of_clone);

      return clone_ptr.release();
   }
   /*virtual*/ void with_target_and_maybe_location::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (auto& opt = this->location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->target.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void with_target_and_maybe_location::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (auto& opt = this->location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->target.clear(my_owner);
   }
}