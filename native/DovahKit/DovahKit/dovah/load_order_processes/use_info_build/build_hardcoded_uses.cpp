#include "./build_hardcoded_uses.h"
#include "dovah/facts/form_ids/hardcoded.h"
#include "dovah/use_info/entry_flags/reference.h"
#include "dovah/form_stub.h"
#include "./use_info_builder.h"

namespace dovah::load_order_processes::use_info_build {
   extern void build_hardcoded_uses(use_info_builder& uib) {
      auto& stub = uib.stub();
      switch (stub.form_id) {
         case facts::form_ids::hardcoded::PlayerRef:
            uib.add_outbound_use(facts::form_ids::hardcoded::Player, use_info::entry_flags::reference::base_form);
            break;
         case facts::form_ids::hardcoded::DefaultWeather:
            //
            // The Creation Kit lists this form as referring to [IMGS:167]DefaultImageSpace, but 
            // when the game initializes both forms, it doesn't actually link them together on 
            // its own. Presumably, it's just Skyrim.esm that does it, though that doesn't mean 
            // it isn't a Creation Kit-level default.
            //
            break;
      }
   }
}