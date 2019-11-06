#pragma once
#include <cstdint>
#include <string>
#include "../types.h"
#include "../components.h"

namespace LoadedForms {
   class Form {
      public:
         const formtype_t formType;
         Form(formtype_t ft) : formType(ft) {};
   };
}