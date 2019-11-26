#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "../components.h"
#include "../papyrus.h"

class TESPluginRecord;

namespace LoadedForms {
   class FormList : public Form {
      public:
         FormList() : Form(FormType::FormList) {};

         std::vector<form_id_t> contents;

         void load(TESPluginRecord&);
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}