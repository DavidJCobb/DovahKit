#include "construct.h"
#include "../FormList.h"
#include "../Location.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../Voicetype.h"

namespace {
   using namespace dovah;

   template<typename T> loaded_forms::Form* _constructAndLoad(TESPluginRecord& record) {
      auto instance = new T;
      instance->load(record);
      return (LoadedForms::Form*) instance;
   }

   struct _Builder {
      form_type_t formType;
      loaded_form_factory_t builder;

      _Builder(form_type_t f, loaded_form_factory_t b) : formType(f), builder(b) {}
   };
   _Builder _builders[] = {
      { form_type::quest,     _constructAndLoad<loaded_forms::Quest> },
      { form_type::formlist,  _constructAndLoad<loaded_forms::FormList> },
      { form_type::voicetype, _constructAndLoad<loaded_forms::Voicetype> },
      { form_type::location,  _constructAndLoad<loaded_forms::Location> },
      { form_type::shout,     _constructAndLoad<loaded_forms::Shout> },
   };
}
namespace dovah {
   loaded_form_factory_t get_loaded_form_factory_by_type(form_type_t ft) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.formType == ft)
            return b.builder;
      }
      return nullptr;
   }
}