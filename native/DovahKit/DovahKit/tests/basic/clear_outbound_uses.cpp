#include "./clear_outbound_uses.h"
#include "dovah/active_load_order.h"

#include "../pieces/load_skyrim_esm.h"

#include "dovah/form_data/operations/clear_outbound_uses.h"
#include "dovah/form_data/types/s/shout.h"

namespace dovahkit::tests {
   extern void clear_outbound_uses() {
      auto alo_ptr = pieces::load_skyrim_esm(false);
      if (!alo_ptr)
         return;
      auto& alo = *alo_ptr;
      
      {  // Become Ethereal
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x32920);
         auto  ptr  = stub->load_as_type<dovah::form_data::shout>();
         __debugbreak();
         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this
         __debugbreak();
      }
      {  // Dismay
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x2395a);
         auto  ptr  = stub->load_as_type<dovah::form_data::shout>();
         __debugbreak();
         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this
         __debugbreak();
      }
   }
}