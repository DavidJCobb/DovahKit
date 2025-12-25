#include "./refs_need_persistence_checker.h"
#include <cassert>
#include <stdexcept>
#include "../data/hardcoded_form_ids.h"
#include "../files/file_load_order.h"
#include "../form_stub.h"
#include "../form_stubs/helpers/get_base_form.h"

#include "../forms/factories/hardcoded.h"
#include "../forms/ObjectReference.h"

#include "../forms/components/extra_data/types/l/location.h"
#include "../forms/Activator.h"
#include "../forms/ActorBase.h"
#include "../forms/DefaultObjectManager.h"
#include "../forms/Door.h"
#include "../forms/Quest.h"

namespace {
   constexpr bool handle_unique_actors_conservatively = true;
}

namespace dovah {
   refs_need_persistence_checker::refs_need_persistence_checker(file_load_order& lo) : load_order(lo) {
   }

   void refs_need_persistence_checker::_grab_default_objects() {
      if (this->grabbed_dobj)
         return;
      this->grabbed_dobj = true;

      auto* dobj_stub = this->load_order.get_canonical_instance_of_singleton_form(dovah::form_type::default_object_manager);
      if (!dobj_stub)
         return;
      auto loaded = dobj_stub->load_even_if_unsafe({}).ptr_cast<loaded_forms::DefaultObjectManager>();
      if (!loaded)
         return;
      
      this->dobj.dragon_marker_crash = loaded->get_entry('DCZM');
      this->dobj.dragon_marker_land  = loaded->get_entry('DLZM');
      this->dobj.persist_all         = loaded->get_entry('PLOC');
   }

   /*static*/ bool refs_need_persistence_checker::_actor_is_unique(dovah::form_stub& actor_base) {
      auto loaded = actor_base.load_even_if_unsafe({}).ptr_cast<dovah::loaded_forms::ActorBase>();
      if (loaded)
         if (loaded->actor_flags & dovah::loaded_forms::ActorBase::actor_flag::unique)
            return true;
      return true;
   }

   /*static*/ bool refs_need_persistence_checker::_quest_targets_unique_actor(dovah::form_stub& quest_stub, const dovah::form_stub& unique_actor_base_stub) {
      auto loaded = quest_stub.load_even_if_unsafe({}).ptr_cast<dovah::loaded_forms::Quest>();
      if (!loaded)
         return false;

      for (const auto* item : loaded->aliases) {
         using namespace dovah::loaded_forms;
         if (item->type != Alias::alias_type::reference)
            continue;

         const auto* casted = (const ReferenceAlias*) item;
         if (auto* data = std::get_if<structs::alias_fill_params::ref::unique_actor>(&casted->fill_params)) {
            if (data->actor_base == &unique_actor_base_stub)
               return true;
         }
      }

      return false;
   }

   bool refs_need_persistence_checker::check_ref(form_stub& refr) {
      if (!this->during_save) {
         if (this->load_order.is_form_loading_blocked(&refr)) {
            throw std::logic_error("cannot properly check REFR persistence requirements during load/save, or when form loading is not possible generally");
         }
      }

      if (!refr.inbound.empty())
         return true;

      auto* base_stub = form_stub_helpers::get_base_form(&refr);
      if (!base_stub)
         return false;

      switch (base_stub->formID) {
         case hardcoded_form_ids::PrisonMarker:
         case hardcoded_form_ids::DivineMarker:
         case hardcoded_form_ids::TempleMarker:
         case hardcoded_form_ids::MapMarker:
         case hardcoded_form_ids::HorseMarker:
         case hardcoded_form_ids::MultiBoundMarker:
         case hardcoded_form_ids::RoomMarker:
         case hardcoded_form_ids::XMarkerHeading:
         case hardcoded_form_ids::XMarker:
            return true;
      }

      switch (base_stub->form_type) {
         case dovah::form_type::activator:
            for (const auto& item : base_stub->outbound) {
               if (item.second.flags & use_info_entry::flag::water_acti_type)
                  return true;
            }
            break;
         case dovah::form_type::actor_base:
            if (_actor_is_unique(*base_stub)) {
               if constexpr (handle_unique_actors_conservatively) {
                  for (const auto& item : base_stub->inbound) {
                     auto* referrer = item.second.other;
                     assert(referrer);
                     if (referrer->form_type == form_type::quest)
                        if (_quest_targets_unique_actor(*referrer, *base_stub))
                           return true;
                  }
               } else {
                  return true;
               }
            }
            break;
         case dovah::form_type::door:
            {
               auto loaded = base_stub->load_even_if_unsafe({}).ptr_cast<loaded_forms::Door>();
               if (loaded)
                  if (!loaded->random_destinations.empty())
                     return true;
            }
            break;
         case dovah::form_type::light:
            if (refr.test_record_flags(loaded_forms::ObjectReference::form_flag::never_fades))
               return true;
            break;
         case dovah::form_type::texture_set:
            return true;
      }

      this->_grab_default_objects();

      if (base_stub == this->dobj.dragon_marker_crash || base_stub == this->dobj.dragon_marker_land)
         return true;

      if (auto* ploc = this->dobj.persist_all) {
         if (refr.outbound.contains(ploc->formID)) {
            auto loaded_refr = refr.load_even_if_unsafe({}).ptr_cast<loaded_forms::ObjectReference>();
            if (loaded_refr) {
               auto* extra = loaded_refr->extra_data.get<loaded_forms::components::extra_data_types::location>();
               if (extra && extra->form == ploc)
                  return true;
            }
         }
      }

      return false;
   }

   void refs_need_persistence_checker::_set_is_during_save(file_writing_passkey) {
      this->during_save = true;
   }
}