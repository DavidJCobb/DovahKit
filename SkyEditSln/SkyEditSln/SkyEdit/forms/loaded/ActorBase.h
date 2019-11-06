#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "../components.h"
#include "../papyrus.h"

class TESPluginRecord;

namespace LoadedForms {
   class ActorBase : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         ActorBase() : Form(FormType::ActorBase) {};

         std::string editorID;
         LStringRef  name;

         void load(TESPluginRecord&);
   };
}