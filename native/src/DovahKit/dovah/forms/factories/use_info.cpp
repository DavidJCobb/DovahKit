#include "use_info.h"
#include "../Activator.h"
#include "../Actor.h"
#include "../ActorBase.h"
#include "../Cell.h"
#include "../Color.h"
#include "../Container.h"
#include "../DefaultObjectManager.h"
#include "../DialogueBranch.h"
#include "../Faction.h"
#include "../FormList.h"
#include "../Landscape.h"
#include "../LandTexture.h"
#include "../Location.h"
#include "../MagicEffect.h"
#include "../Note.h"
#include "../ObjectReference.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../Topic.h"
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
      { form_type::faction,                dovah::loaded_forms::Faction::generate_use_info },
      { form_type::magic_effect,           dovah::loaded_forms::MagicEffect::generate_use_info },
      { form_type::land_texture,           dovah::loaded_forms::LandTexture::generate_use_info },
      { form_type::activator,              dovah::loaded_forms::Activator::generate_use_info },
      { form_type::container,              dovah::loaded_forms::Container::generate_use_info },
      { form_type::actor_base,             dovah::loaded_forms::ActorBase::generate_use_info },
      { form_type::note,                   dovah::loaded_forms::Note::generate_use_info },
      { form_type::cell,                   dovah::loaded_forms::Cell::generate_use_info },
      { form_type::reference,              dovah::loaded_forms::ObjectReference::generate_use_info },
      { form_type::actor,                  dovah::loaded_forms::Actor::generate_use_info },
      { form_type::worldspace,             dovah::loaded_forms::Worldspace::generate_use_info },
      { form_type::land,                   dovah::loaded_forms::Landscape::generate_use_info },
      { form_type::topic,                  dovah::loaded_forms::Topic::generate_use_info },
      { form_type::topic_info,             dovah::loaded_forms::TopicInfo::generate_use_info },
      { form_type::quest,                  dovah::loaded_forms::Quest::generate_use_info },
      { form_type::formlist,               dovah::loaded_forms::FormList::generate_use_info },
      { form_type::voicetype,              dovah::loaded_forms::Voicetype::generate_use_info },
      { form_type::location,               dovah::loaded_forms::Location::generate_use_info },
      { form_type::default_object_manager, dovah::loaded_forms::DefaultObjectManager::generate_use_info },
      { form_type::dialogue_branch,        dovah::loaded_forms::DialogueBranch::generate_use_info },
      { form_type::shout,                  dovah::loaded_forms::Shout::generate_use_info },
      { form_type::word_of_power,          dovah::loaded_forms::WordOfPower::generate_use_info },
      { form_type::color,                  dovah::loaded_forms::Color::generate_use_info },
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