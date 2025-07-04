#include "./with_target.h"
#include <memory>
#include "../../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info::generic {
   /*virtual*/ void with_target::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
   }
   /*virtual*/ void with_target::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
   }
   /*virtual*/ base* with_target::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<with_target>();
      auto* clone     = clone_ptr.get();

      clone->target.clone_from(this->target, owner_of_clone);

      return clone_ptr.release();
   }
   /*virtual*/ void with_target::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->target.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void with_target::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->target.clear(my_owner);
   }
}