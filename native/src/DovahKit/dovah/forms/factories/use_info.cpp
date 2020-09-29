#include "use_info.h"
#include "../Activator.h"
#include "../Actor.h"
#include "../ActorBase.h"
#include "../Cell.h"
#include "../Color.h"
#include "../Container.h"
#include "../Faction.h"
#include "../FormList.h"
#include "../Location.h"
#include "../MagicEffect.h"
#include "../ObjectReference.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../TopicInfo.h"
#include "../Voicetype.h"
#include "../WordOfPower.h"
#include "../Worldspace.h"

namespace {
   using namespace dovah;

   struct _Builder {
      form_type_t form_type;
      outbound_uses_builder_t builder;

      _Builder(form_type_t f, outbound_uses_builder_t b) : form_type(f), builder(b) {}
   };
   _Builder _builders[] = {
      { form_type::faction,       dovah::loaded_forms::Faction::generateUseInfo },
      { form_type::magic_effect,  dovah::loaded_forms::MagicEffect::generateUseInfo },
      { form_type::activator,     dovah::loaded_forms::Activator::generateUseInfo },
      { form_type::container,     dovah::loaded_forms::Container::generateUseInfo },
      { form_type::actor_base,    dovah::loaded_forms::ActorBase::generateUseInfo },
      { form_type::cell,          dovah::loaded_forms::Cell::generateUseInfo },
      { form_type::reference,     dovah::loaded_forms::ObjectReference::generateUseInfo },
      { form_type::actor,         dovah::loaded_forms::Actor::generateUseInfo },
      { form_type::worldspace,    dovah::loaded_forms::Worldspace::generateUseInfo },
      { form_type::topic_info,    dovah::loaded_forms::TopicInfo::generateUseInfo },
      { form_type::quest,         dovah::loaded_forms::Quest::generateUseInfo },
      { form_type::formlist,      dovah::loaded_forms::FormList::generateUseInfo },
      { form_type::voicetype,     dovah::loaded_forms::Voicetype::generateUseInfo },
      { form_type::location,      dovah::loaded_forms::Location::generateUseInfo },
      { form_type::shout,         dovah::loaded_forms::Shout::generateUseInfo },
      { form_type::word_of_power, dovah::loaded_forms::WordOfPower::generateUseInfo },
      { form_type::color,         dovah::loaded_forms::Color::generateUseInfo },
   };
}
namespace dovah {
   outbound_uses_builder_t get_outbound_uses_builder_by_type(form_type_t ft) noexcept {
      for (uint32_t i = 0; i < std::extent<decltype(_builders)>::value; i++) {
         auto& b = _builders[i];
         if (b.form_type == ft)
            return b.builder;
      }
      return nullptr;
   }
}