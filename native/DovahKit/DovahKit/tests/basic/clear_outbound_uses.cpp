#include "./clear_outbound_uses.h"
#include "dovah/active_load_order.h"

#include "../pieces/load_skyrim_esm.h"
//
#include "../pieces/assert_uses_are_severed.h"
#include "../pieces/get_expected_used_stubs.h"

#include "dovah/form_data/operations/clear_outbound_uses.h"
#include "dovah/form_data/types/s/shout.h"

namespace dovahkit::tests {
   extern void clear_outbound_uses() {
      auto alo_ptr = pieces::load_skyrim_esm(false);
      if (!alo_ptr)
         return;
      auto& alo = *alo_ptr;
      
      {  // Become Ethereal
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x00032920);
         auto  ptr  = stub->load_as_type<dovah::form_data::shout>();
         __debugbreak();

         auto used_stubs = pieces::get_expected_used_stubs<7>(*stub, {
            0x00032917, // [WOOP]WordFeim
            0x00032918, // [WOOP]WordZii
            0x00032919, // [WOOP]WordGron
            0x0005F6EB, // [SPEL]VoiceBecomeEthereal1
            0x0005F6EC, // [SPEL]VoiceBecomeEthereal2
            0x0005F6ED, // [SPEL]VoiceBecomeEthereal3
            0x0010EA49, // [STAT]MAGINVShoutEtherial
         });

         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this

         pieces::assert_uses_are_severed(*stub, used_stubs);
         __debugbreak();
      }
      {  // Dismay
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x0002395A);
         auto  ptr  = stub->load_as_type<dovah::form_data::shout>();
         __debugbreak();

         auto used_stubs = pieces::get_expected_used_stubs<7>(*stub, {
            0x0002395B, // [SPEL]VoiceDismayingShout1
            0x0002395E, // [SPEL]VoiceDismayingShout2
            0x00023967, // [SPEL]VoiceDismayingShout3
            0x0003291A, // [WOOP]WordFaas
            0x0003291B, // [WOOP]WordRu
            0x0003291C, // [WOOP]WordMaar
            0x0010EA47, // [STAT]MAGINVShoutPinkMagic
         });

         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this

         pieces::assert_uses_are_severed(*stub, used_stubs);
         __debugbreak();
      }
   }
}