#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "../components.h"
#include "../container.h"
#include "../model.h"
#include "../papyrus.h"

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