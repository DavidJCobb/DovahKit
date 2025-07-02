#include "./patrol.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void patrol::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      subrecord.read(this->repeatable);
      subrecord.skip_bytes(1);
   }
   /*virtual*/ void patrol::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      {
         auto& subrecord = record.open_next_subrecord(header_subrecord);
         subrecord.write(this->repeatable);
         subrecord.skip_bytes(1);
         subrecord.close();
      }
      {
         auto& loc = this->location;
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* patrol::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<patrol>();
      auto* clone     = clone_ptr.get();

      clone->location.clone_from(this->location, owner_of_clone);

      clone->repeatable = this->repeatable;

      return clone_ptr.release();
   }
   /*virtual*/ void patrol::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->location.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void patrol::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->location.clear(my_owner);

      this->repeatable = false;
   }
