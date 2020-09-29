#include "construct.h"
#include "../Actor.h"
#include "../Cell.h"
#include "../Color.h"
#include "../Faction.h"
#include "../FormList.h"
#include "../Location.h"
#include "../ObjectReference.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../Voicetype.h"
#include "../WordOfPower.h"
#include "../Worldspace.h"

namespace {
   using namespace dovah;
   using _loader_t    = loaded_form_load_function_t;
   using _construct_t = loaded_forms::Form*(*)();
}
namespace {
   using namespace dovah;

   template<typename T> loaded_forms::Form* _construct_and_load(tes_record_reader& record) {
      auto instance = new T;
      instance->load(record);
      return (loaded_forms::Form*) instance;
   }
   template<typename T> loaded_forms::Form* _construct() {
      return new T;
   }

   struct _handlers {
      _loader_t    builder     = nullptr;
      _construct_t constructor = nullptr;

      template<typename T> static _handlers make() {
         _handlers instance;
         instance.builder     = _construct_and_load<T>;
         instance.constructor = _construct<T>;
         return instance;
      }
   };

   struct _entry {
      form_type_t form_type;
      _handlers   handlers;
   };

   _entry _builders[] = {
      { form_type::faction,       _handlers::make<loaded_forms::Faction>() },
      { form_type::cell,          _handlers::make<loaded_forms::Cell>() },
      { form_type::reference,     _handlers::make<loaded_forms::ObjectReference>() },
      { form_type::actor,         _handlers::make<loaded_forms::Actor>() },
      { form_type::worldspace,    _handlers::make<loaded_forms::Worldspace>() },
      { form_type::quest,         _handlers::make<loaded_forms::Quest>() },
      { form_type::formlist,      _handlers::make<loaded_forms::FormList>() },
      { form_type::voicetype,     _handlers::make<loaded_forms::Voicetype>() },
      { form_type::location,      _handlers::make<loaded_forms::Location>() },
      { form_type::shout,         _handlers::make<loaded_forms::Shout>() },
      { form_type::word_of_power, _handlers::make<loaded_forms::WordOfPower>() },
      { form_type::color,         _handlers::make<loaded_forms::Color>() },
   };
}
namespace dovah {
   loaded_form_load_function_t get_loaded_form_factory_by_type(form_type_t ft) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.form_type == ft)
            return b.handlers.builder;
      }
      return nullptr;
   }
   loaded_forms::Form* create_blank_loaded_form_by_type(form_type_t ft) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.form_type == ft)
            return (b.handlers.constructor)();
      }
      return nullptr;
   }
}