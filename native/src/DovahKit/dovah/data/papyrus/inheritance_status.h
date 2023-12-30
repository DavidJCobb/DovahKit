#pragma once

namespace dovah::papyrus {
   struct inheritance_status {
      bool present_on_base   = false;
      bool present_on_target = false;
      bool removed_on_target = false;
      bool present_in_script = false; // for properties: is it present in the compiled PEX?
   };
}