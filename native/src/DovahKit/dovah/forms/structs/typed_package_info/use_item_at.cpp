#include "./use_item_at.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void use_item_at::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;
   }
   /*virtual*/ void use_item_at::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      record.open_next_subrecord(header_subrecord).close();
      if (auto& opt = this->search_location; opt.has_value()) {
         auto& loc = opt.value();
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* use_item_at::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<use_item_at>();
      auto* clone     = clone_ptr.get();

      clone->use_at_location.clone_from(this->use_at_location, owner_of_clone);
      {
         auto& src = this->search_location;
         auto& dst = clone->search_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->item_to_use.clone_from(this->item_to_use, owner_of_clone);

      return clone_ptr.release();
   }
   /*virtual*/ void use_item_at::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->use_at_location.sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->search_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->item_to_use.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void use_item_at::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->use_at_location.clear(my_owner);
      if (auto& opt = this->search_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->item_to_use.clear(my_owner);
   }
}