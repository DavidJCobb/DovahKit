#include "./escort.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void escort::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      subrecord.read(this->distance);
   }
   /*virtual*/ void escort::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write(this->distance);
      subrecord.close();
      if (auto& opt = this->search_location; opt.has_value()) {
         auto& loc = opt.value();
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* escort::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<escort>();
      auto* clone     = clone_ptr.get();

      {
         auto& src = this->destination;
         auto& dst = clone->destination;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      {
         auto& src = this->search_location;
         auto& dst = clone->search_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->target.clone_from(this->target, owner_of_clone);

      clone->distance = this->distance;

      return clone_ptr.release();
   }
   /*virtual*/ void escort::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (auto& opt = this->destination; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->search_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->target.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void escort::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (auto& opt = this->destination; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      if (auto& opt = this->search_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->target.clear(my_owner);

      this->distance = 0;
   }
}