#include "./with_maybe_each.h"
#include <memory>
#include "../../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info::generic {
   /*virtual*/ void with_maybe_each::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
   }
   /*virtual*/ void with_maybe_each::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
   }
   /*virtual*/ base* with_maybe_each::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<with_maybe_each>(this->type);
      auto* clone     = clone_ptr.get();
      
      {
         auto& src = this->location;
         auto& dst = clone->location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      {
         auto& src = this->target;
         auto& dst = clone->target;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }

      return clone_ptr.release();
   }
   /*virtual*/ void with_maybe_each::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (auto& opt = this->location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->target; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void with_maybe_each::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (auto& opt = this->location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      if (auto& opt = this->target; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
   }
}