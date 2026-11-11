#include "./clear_outbound_uses.h"
#include "dovah/active_load_order.h"

#include "../pieces/load_skyrim_esm.h"
//
#include "../pieces/assert_even_in_release.h"
#include "../pieces/assert_uses_are_severed.h"
#include "../pieces/assert_uses_strings.h"
#include "../pieces/get_expected_used_stubs.h"

#include "dovah/form_data/operations/clear_outbound_uses.h"
#include "dovah/form_data/types/f/FormList.h"
#include "dovah/form_data/types/s/Shout.h"

namespace dovahkit::tests {
   extern void clear_outbound_uses() {
      auto alo_ptr = pieces::load_skyrim_esm(false);
      if (!alo_ptr)
         return;
      auto& alo = *alo_ptr;
      
      {  // Become Ethereal
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x00032920);
         auto  ptr  = stub->load_as_type<dovah::form_data::Shout>();
         __debugbreak();

         const auto used_stubs = pieces::get_expected_used_stubs<7>(*stub, {
            0x00032917, // [WOOP]WordFeim
            0x00032918, // [WOOP]WordZii
            0x00032919, // [WOOP]WordGron
            0x0005F6EB, // [SPEL]VoiceBecomeEthereal1
            0x0005F6EC, // [SPEL]VoiceBecomeEthereal2
            0x0005F6ED, // [SPEL]VoiceBecomeEthereal3
            0x0010EA49, // [STAT]MAGINVShoutEtherial
         });
         pieces::assert_uses_strings<2>(*stub, {
            types::localized_string_descriptor{ "Skyrim.esm", dovah::localization::category::description, 0x3CA },
            types::localized_string_descriptor{ "Skyrim.esm", dovah::localization::category::general,     0x3CD },
         });

         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this

         pieces::assert_uses_are_severed(*stub, used_stubs);
         assert_even_in_release(stub->get_string_uses().empty() && "String uses should've been cleared!");
         __debugbreak();
      }

      {  // Dismay
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x0002395A);
         auto  ptr  = stub->load_as_type<dovah::form_data::Shout>();
         __debugbreak();

         const auto used_stubs = pieces::get_expected_used_stubs<7>(*stub, {
            0x0002395B, // [SPEL]VoiceDismayingShout1
            0x0002395E, // [SPEL]VoiceDismayingShout2
            0x00023967, // [SPEL]VoiceDismayingShout3
            0x0003291A, // [WOOP]WordFaas
            0x0003291B, // [WOOP]WordRu
            0x0003291C, // [WOOP]WordMaar
            0x0010EA47, // [STAT]MAGINVShoutPinkMagic
         });
         pieces::assert_uses_strings<2>(*stub, {
            types::localized_string_descriptor{ "Skyrim.esm", dovah::localization::category::description, 0x43B },
            types::localized_string_descriptor{ "Skyrim.esm", dovah::localization::category::general,     0x43C },
         });

         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this

         pieces::assert_uses_are_severed(*stub, used_stubs);
         assert_even_in_release(stub->get_string_uses().empty() && "String uses should've been cleared!");
         __debugbreak();
      }

      {  // MS07NPCs
         auto* stub = alo.get_form_stub_collection().get_form_by_id(0x00025F7A);
         auto  ptr  = stub->load_as_type<dovah::form_data::FormList>();
         __debugbreak();

         const auto used_stubs = pieces::get_expected_used_stubs<5>(*stub, {
            0x00025F8A, // [NPC_]MS07Bandit03Melee1H
            0x00013298, // [NPC_]Mazaka
            0x000132A1, // [NPC_]Rikke
            0x00013268, // [NPC_]Deeja
            0x0001328D, // [NPC_]JareeRa
         });

         dovah::form_data_operations::clear_outbound_uses(*stub, *ptr);
         stub->set_edited(true); // `clear_outbound_uses` does not implicitly call this

         pieces::assert_uses_are_severed(*stub, used_stubs);
         __debugbreak();
      }
   }
}