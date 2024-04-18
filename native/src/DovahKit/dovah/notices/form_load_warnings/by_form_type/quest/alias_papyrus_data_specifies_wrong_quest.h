#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::quest {
   //
   // The VMAD subrecord for one quest can actually attach scripts to aliases on a 
   // completely different quest, by specifying those quests in the header for a 
   // quest alias's script data. I don't know if this is intentional (nor do I know 
   // why it *would* be), and Bethesda's tools never make use of it.
   // 
   // This is an unfortunate case where the data is technically ill-formed, but the 
   // game actually has no problem loading it; we, however, do. We load forms' full 
   // data completely independently of one another, so we can't properly handle this. 
   // At least, not without adding some major special-case code for this deep into the 
   // loader, wherein some quest B can be aware that it has scripts remotely attached 
   // to its aliases by a VMAD subrecord in some quest A which will have to be fetched 
   // and loaded... For now, we don't attempt to support it.
   //
   class alias_papyrus_data_specifies_wrong_quest : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr alias_papyrus_data_specifies_wrong_quest(
            form_stub& subject,
            form_stub& target,
            uint16_t   alias
         )
         :
            base_form_load_warning(subject),
            target(target),
            target_alias_id(alias)
         {}

         form_stub& target; // quest that would have a script remotely attached to one of its aliases
         uint16_t   target_alias_id;
   };
}
#include "../../../_util.undef.h"