#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::leveled_list {
   //
   // In Skyrim, when a leveled list form loads a list entry (LVLO), it stores a 
   // pointer to that list entry in a static variable for use by any subsequent 
   // COED subrecord. The handler for the COED subrecord will check if the pointer 
   // is non-null and if so, load to that list entry.
   // 
   // The problem is this: Bethesda never clears the static variable at the start 
   // nor the end of the functions for loading leveled lists. I suspect that they 
   // mistakenly thought that the following two constructions are equivalent:
   // 
   //    static int foo = 0; // initializer i.e. only writes the first time
   // 
   //    static int bar = 0; // initializer i.e. only writes the first time
   //    bar = 0;            // writes every time
   // 
   // The result of this problem is that there's a "hole" in the loader. Suppose 
   // the game loads two leveled lists of the same form type A and B. If the last 
   // LVLO in A has no corresponding COED, and if B has a COED before its first 
   // LVLO, then the COED subrecord in B will load to the last leveled object in 
   // A -- a cross-form load!
   // 
   // This seems to affect all leveled list forms. Forms load by switch-casing 
   // each subrecord's signature, so I'm betting Bethesda has macros for each form 
   // component. Leveled list forms have almost identical loaders.
   //
   class leading_coed_bleedthrough : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr leading_coed_bleedthrough(form_stub& subject) : base_form_load_warning(subject) {}
   };
}
#include "../../../_util.undef.h"