#include "use_info.h"
#include "../loaded/Quest.h"

namespace {
   struct _Builder {
      formtype_t formType;
      FormOutboundUsesBuilder builder;

      _Builder(formtype_t f, FormOutboundUsesBuilder b) : formType(f), builder(b) {}
   };
   _Builder _builders[] = {
      { FormType::Quest, LoadedForms::Quest::generateUseInfo },
   };
}
FormOutboundUsesBuilder getOutboundUsesBuilderForFormType(formtype_t ft) noexcept {
   for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
      auto& b = _builders[i];
      if (b.formType == ft)
         return b.builder;
   }
   return nullptr;
}