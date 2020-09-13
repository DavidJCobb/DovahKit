#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Activator : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         Activator() : Form(form_type::activator) {};

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}