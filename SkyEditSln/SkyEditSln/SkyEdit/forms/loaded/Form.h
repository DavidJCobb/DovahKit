#pragma once
#include <cstdint>
#include <string>
#include "../types.h"
#include "../components.h"

#define LOAD_NAIVELY_WHEN_THE_GAME_DOES 1

class FormStub;
namespace LoadedForms {
   class Form {
      public:
         const formtype_t formType;
         Form(formtype_t ft) : formType(ft) {};
         //
         FormStub* stub = nullptr;
   };
}