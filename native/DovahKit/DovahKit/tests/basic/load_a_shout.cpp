#include "./load_a_shout.h"
#include "dovah/active_load_order.h"

#include "../pieces/load_skyrim_esm.h"

namespace dovahkit::tests {
   extern void load_a_shout() {
      auto alo_ptr = pieces::load_skyrim_esm(true);
      if (!alo_ptr)
         return;
      auto& alo = *alo_ptr;
      
      {  // Become Ethereal
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x32920);
         auto  ptr  = stub->load();
         __debugbreak();
      }
      {  // Dismay
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x2395a);
         auto  ptr  = stub->load();
         __debugbreak();
      }
   }
}