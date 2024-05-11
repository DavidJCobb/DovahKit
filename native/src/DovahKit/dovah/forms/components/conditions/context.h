#pragma once
#include "../../../form_stub.h"

namespace dovah {
   namespace loaded_forms {
      class Package;
      class Quest;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::conditions {
   //
   // Helper struct for working with conditions.
   //
   struct context {
      public:
         context() {}
         context(form_stub&, bool prefer_working_copy = true);
         
      public:
         form_stub* owner   = nullptr; // the form that contains the conditions
         form_stub* package = nullptr; // (owner) if it's a PACK, or its owning PACK
         form_stub* quest   = nullptr; // (owner) if it's a QUST, or its owning QUST
         struct {
            loaded_form_ptr<loaded_forms::Package> package;
            loaded_form_ptr<loaded_forms::Quest>   quest;
         } loaded;
         bool prefer_working_copy = true;

      public:
         loaded_forms::Package* get_owning_package() const noexcept; // gets the working copy or, if there isn't one, the form
         loaded_forms::Quest*   get_owning_quest() const noexcept; // gets the working copy or, if there isn't one, the form
   };
}