#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ObjectReference : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         ObjectReference() : Form(form_type::reference) {};

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}