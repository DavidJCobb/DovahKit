#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "../components.h"
#include "../container.h"
#include "../papyrus.h"

class TESPluginRecord;

namespace LoadedForms {
   class ActorBase : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         ActorBase() : Form(FormType::ActorBase) {};

         LStringRef  name;

         void load(TESPluginRecord&); // TODO: FINISH ME
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}