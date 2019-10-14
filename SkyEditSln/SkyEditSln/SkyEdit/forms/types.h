#pragma once
#include <cstdint>

typedef uint8_t formtype_t;

struct FormTypeInfo {
   uint32_t    signature;
   uint8_t     formType;
   const char* name;
};

extern FormTypeInfo formTypes[];

extern formtype_t signatureToFormType(uint32_t signature);

class TESForm {
   public:
      const formtype_t formType;
      TESForm(formtype_t ft) : formType(ft) {};
};