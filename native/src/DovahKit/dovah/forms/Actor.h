#pragma once
#include "ObjectReference.h"

namespace dovah::loaded_forms {
   class Actor : public ObjectReference {
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::actor;
         Actor() : ObjectReference(form_type) {};

         void load(tes_record_reader& r) {
            ObjectReference::load(r); // in TESV.exe, Actor doesn't override TESObjectREFR::LoadForm.
         }
         static void generateUseInfo(tes_record_reader& r, form_stub* s) {
            ObjectReference::generateUseInfo(r, s);
         }
   };
}