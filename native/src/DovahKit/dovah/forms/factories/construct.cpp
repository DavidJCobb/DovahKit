#include "construct.h"
#include "../Activator.h"
#include "../Actor.h"
#include "../Cell.h"
#include "../Color.h"
#include "../DefaultObjectManager.h"
#include "../Faction.h"
#include "../FormList.h"
#include "../Location.h"
#include "../ObjectReference.h"
#include "../Package.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../Voicetype.h"
#include "../WordOfPower.h"
#include "../Worldspace.h"

namespace {
   using namespace dovah;
   using _loader_t    = form_loader_function_t;
   using _construct_t = loaded_forms::Form*(*)(const loaded_forms::Form::constructor_params&);
}
namespace {
   using namespace dovah;

   template<typename T> void _load(loaded_forms::Form* instance, tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      ((T*)instance)->load(record, intfc);
   }
   template<typename T> loaded_forms::Form* _construct(const loaded_forms::Form::constructor_params& c) {
      return new T(c);
   }

   struct _handlers {
      _loader_t    loader     = nullptr;
      _construct_t constructor = nullptr;

      template<typename T> static _handlers make() {
         _handlers instance;
         instance.loader      = _load<T>;
         instance.constructor = _construct<T>;
         return instance;
      }
   };

   struct _entry {
      form_type_t form_type;
      _handlers   handlers;
   };

   _entry _builders[] = {
      { form_type::faction,                _handlers::make<loaded_forms::Faction>() },
      { form_type::activator,              _handlers::make<loaded_forms::Activator>() },
      { form_type::cell,                   _handlers::make<loaded_forms::Cell>() },
      { form_type::reference,              _handlers::make<loaded_forms::ObjectReference>() },
      { form_type::actor,                  _handlers::make<loaded_forms::Actor>() },
      { form_type::worldspace,             _handlers::make<loaded_forms::Worldspace>() },
      { form_type::quest,                  _handlers::make<loaded_forms::Quest>() },
      { form_type::formlist,               _handlers::make<loaded_forms::FormList>() },
      { form_type::voicetype,              _handlers::make<loaded_forms::Voicetype>() },
      { form_type::location,               _handlers::make<loaded_forms::Location>() },
      { form_type::default_object_manager, _handlers::make<loaded_forms::DefaultObjectManager>() },
      { form_type::shout,                  _handlers::make<loaded_forms::Shout>() },
      { form_type::word_of_power,          _handlers::make<loaded_forms::WordOfPower>() },
      { form_type::color,                  _handlers::make<loaded_forms::Color>() },
   };
}
namespace dovah {
   form_loader_function_t get_form_loader_function(form_type_t ft) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.form_type == ft)
            return b.handlers.loader;
      }
      return nullptr;
   }
   loaded_forms::Form* create_blank_loaded_form_by_type(form_type_t ft, const loaded_forms::Form::constructor_params& c) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.form_type == ft)
            return (b.handlers.constructor)(c);
      }
      return nullptr;
   }
}