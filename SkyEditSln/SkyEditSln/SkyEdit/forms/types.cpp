#include "types.h"
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'DIAL', 75, "Topic" },
   { 'QUST', 77, "Quest" },
};

formtype_t signatureToFormType(uint32_t signature) {
   for (uint8_t i = 0; i < std::extent<decltype(formTypes)>::value; i++) {
      auto& info = formTypes[i];
      if (info.signature == signature)
         return info.formType;
   }
   return 0;
}