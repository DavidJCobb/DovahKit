#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/container.h"
#include "components/destruction.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ActorBase : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         ActorBase() : Form(form_type::actor_base) {};

         localized_string name;

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}