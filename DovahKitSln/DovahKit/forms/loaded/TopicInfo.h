#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "../components.h"
#include "../components/papyrus.h"

class TESPluginRecord;

namespace LoadedForms {
   class TopicInfo : public Form {
      //
      // Intentionally minimal for now.
      //
      public:
         TopicInfo() : Form(FormType::TopicInfo) {};

         void load(TESPluginRecord&); // TODO: FINISH ME
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}