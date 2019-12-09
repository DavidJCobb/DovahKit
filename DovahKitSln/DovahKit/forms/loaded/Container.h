#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "../components.h"
#include "../components/bounds.h"
#include "../components/container.h"
#include "../components/model.h"
#include "../components/papyrus.h"

class TESPluginRecord;

namespace LoadedForms {
   class Container : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         Container() : Form(FormType::Container) {};

         void load(TESPluginRecord&); // TODO: FINISH ME
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}