#pragma once
#include <cstdint>
#include <type_traits>

namespace dovah {
   class form_stub;
}

namespace dovah {
   struct use_info_entry {
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            parent_child    = 0x01, // the user-form is a child of the used-form
            //
            // The next flags are useful for unique and high-importance relationships between 
            // specific forms. These must be relationships that can only exist once; for example, 
            // a REFR can only have one base form. If the relevant (form_reference_t) is altered, 
            // the flag will be removed.
            //
            // If two relationships can be outbound from the same form but are mutually exclusive, 
            // that alone is not enough to distinguish them, because a form with malformed data 
            // could be loaded. For example, DIAL/BNAM and DIAL/QNAM are mutually exclusive by 
            // virtue of involving different form types, but a file with ill-formed data could 
            // contain a DIAL that points both subrecords at the same form, and so using the same 
            // flag for both subrecords could in that situation cause use info mismanagement should 
            // either subrecord be altered after load.
            //
            object_reference = 0x02, // REFR/NAME: the user-form is a reference and the used-form is its base form
            dialogue_branch  = 0x04, // DIAL/BNAM
            dialogue_quest   = 0x08, // DIAL/QNAM and DLBR/QNAM
            water_acti_type  = 0x10, // ACTI/WNAM: a water activator's water type
            template_actor   = 0x20, // NPC_/TPLT: the used-form is the user-form's template actor
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      form_stub* other    = nullptr;
      uint32_t   refcount = 0;
      flags_t    flags    = 0;
   };
}