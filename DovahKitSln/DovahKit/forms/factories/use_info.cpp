#include "use_info.h"
#include "../loaded/ActorBase.h"
#include "../loaded/Container.h"
#include "../loaded/FormList.h"
#include "../loaded/MagicEffect.h"
#include "../loaded/ObjectReference.h"
#include "../loaded/Quest.h"
#include "../loaded/TopicInfo.h"
#include "../loaded/Voicetype.h"

namespace {
   struct _Builder {
      formtype_t formType;
      FormOutboundUsesBuilder builder;

      _Builder(formtype_t f, FormOutboundUsesBuilder b) : formType(f), builder(b) {}
   };
   _Builder _builders[] = {
      { FormType::MagicEffect, LoadedForms::MagicEffect::generateUseInfo },
      { FormType::Container,   LoadedForms::Container::generateUseInfo },
      { FormType::ActorBase,   LoadedForms::ActorBase::generateUseInfo },
      { FormType::Reference,   LoadedForms::ObjectReference::generateUseInfo },
      { FormType::TopicInfo,   LoadedForms::TopicInfo::generateUseInfo },
      { FormType::Quest,       LoadedForms::Quest::generateUseInfo },
      { FormType::FormList,    LoadedForms::FormList::generateUseInfo },
      { FormType::Voicetype,   LoadedForms::Voicetype::generateUseInfo },
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