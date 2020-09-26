#include "construct.h"
#include "../Actor.h"
#include "../Cell.h"
#include "../Color.h"
#include "../FormList.h"
#include "../Location.h"
#include "../ObjectReference.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../Voicetype.h"
#include "../WordOfPower.h"

namespace {
   using namespace dovah;

   template<typename T> loaded_forms::Form* _constructAndLoad(tes_record_reader& record) {
      auto instance = new T;
      instance->load(record);
      return (loaded_forms::Form*) instance;
   }

   struct _Builder {
      form_type_t formType;
      loaded_form_factory_t builder;

      _Builder(form_type_t f, loaded_form_factory_t b) : formType(f), builder(b) {}
   };
   _Builder _builders[] = {
      { form_type::cell,          _constructAndLoad<loaded_forms::Cell> },
      { form_type::reference,     _constructAndLoad<loaded_forms::ObjectReference> },
      { form_type::actor,         _constructAndLoad<loaded_forms::Actor> },
      { form_type::quest,         _constructAndLoad<loaded_forms::Quest> },
      { form_type::formlist,      _constructAndLoad<loaded_forms::FormList> },
      { form_type::voicetype,     _constructAndLoad<loaded_forms::Voicetype> },
      { form_type::location,      _constructAndLoad<loaded_forms::Location> },
      { form_type::shout,         _constructAndLoad<loaded_forms::Shout> },
      { form_type::word_of_power, _constructAndLoad<loaded_forms::WordOfPower> },
      { form_type::color,         _constructAndLoad<loaded_forms::Color> },
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