#include "./eat.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void eat::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
   }
   /*virtual*/ void eat::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      record.open_next_subrecord(header_subrecord).close();
      if (auto& opt = this->search_location; opt.has_value()) {
         auto& loc = opt.value();
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* eat::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<eat>();
      auto* clone     = clone_ptr.get();

      clone->eat_location.clone_from(this->eat_location, owner_of_clone);
      {
         auto& src = this->search_location;
         auto& dst = clone->search_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->desired_food.clone_from(this->desired_food, owner_of_clone);

      return clone_ptr.release();
   }
   /*virtual*/ void eat::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->eat_location.sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->search_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->desired_food.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void eat::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->eat_location.clear(my_owner);
      if (auto& opt = this->search_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->desired_food.clear(my_owner);
   }
}