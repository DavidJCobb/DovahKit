#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::container {
   //
   // Items in containers can have extra-data attached via the COED subrecord. 
   // One of the extra-data types is the item's owner, with successive fields' 
   // meanings depending on the owner's form type. If the form type isn't one 
   // of the valid form types, then we have no idea how to read that data.
   //
   class item_has_bad_owner_form_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr item_has_bad_owner_form_type(
            form_stub& subject,
            form_stub& item_owner
         )
         :
            base_form_load_warning(subject),
            item_owner(item_owner)
         {}

         form_stub& item_owner;
   };
}
#include "../../../_util.undef.h"
