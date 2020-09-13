#include "use_info.h"
#include "../Activator.h"
#include "../ActorBase.h"
#include "../Container.h"
#include "../FormList.h"
#include "../Location.h"
#include "../MagicEffect.h"
#include "../ObjectReference.h"
#include "../Quest.h"
#include "../Shout.h"
#include "../TopicInfo.h"
#include "../Voicetype.h"

namespace {
   using namespace dovah;

   struct _Builder {
      form_type_t form_type;
      outbound_uses_builder_t builder;

      _Builder(form_type_t f, outbound_uses_builder_t b) : form_type(f), builder(b) {}
   };
   _Builder _builders[] = {
      { form_type::magic_effect, LoadedForms::MagicEffect::generateUseInfo },
      { form_type::activator,    LoadedForms::Activator::generateUseInfo },
      { form_type::container,    LoadedForms::Container::generateUseInfo },
      { form_type::actor_base,   LoadedForms::ActorBase::generateUseInfo },
      { form_type::reference,    LoadedForms::ObjectReference::generateUseInfo },
      { form_type::topic_info,   LoadedForms::TopicInfo::generateUseInfo },
      { form_type::quest,        LoadedForms::Quest::generateUseInfo },
      { form_type::formlist,     LoadedForms::FormList::generateUseInfo },
      { form_type::voicetype,    LoadedForms::Voicetype::generateUseInfo },
      { form_type::location,     LoadedForms::Location::generateUseInfo },
      { form_type::shout,        LoadedForms::Shout::generateUseInfo },
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