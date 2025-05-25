#include "./MagicEffectSummonableActorPickerFilter.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/actor_base.h"
#include "editor/subsystems/form_info_cache/core.h"

/*virtual*/ bool MagicEffectSummonableActorPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
   auto* info = fic.get_actor_base_info(stub);
   if (!info)
      return true;
   return info->summonable;
}