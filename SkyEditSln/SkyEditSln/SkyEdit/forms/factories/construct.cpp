#include "construct.h"
#include "../loaded/FormList.h"
#include "../loaded/Quest.h"
#include "../loaded/Voicetype.h"

namespace {
   template<typename T> LoadedForms::Form* _constructAndLoad(TESPluginRecord& record) {
      auto instance = new T;
      instance->load(record);
      return (LoadedForms::Form*) instance;
   }

   struct _Builder {
      formtype_t formType;
      LoadedFormFactory builder;

      _Builder(formtype_t f, LoadedFormFactory b) : formType(f), builder(b) {}
   };
   _Builder _builders[] = {
      { FormType::Quest,     _constructAndLoad<LoadedForms::Quest> },
      { FormType::FormList,  _constructAndLoad<LoadedForms::FormList> },
      { FormType::Voicetype, _constructAndLoad<LoadedForms::Voicetype> },
   };
}
LoadedFormFactory getLoadedFormFactoryForFormType(formtype_t ft) noexcept {
   for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
      auto& b = _builders[i];
      if (b.formType == ft)
         return b.builder;
   }
   return nullptr;
}