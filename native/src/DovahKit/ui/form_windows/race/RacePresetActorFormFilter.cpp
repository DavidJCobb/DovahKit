#include "./RacePresetActorFormFilter.h"
#include "dovah/form_stub.h"

#include "dovah/exceptions/actor_base_template_is_cyclical.h"
#include "dovah/forms/ActorBase.h"
#include "dovah/utils/get_all_relevant_template_actors.h"

/*virtual*/ bool RacePresetActorFormFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   if (stub.form_type != dovah::form_type::actor_base)
      return false;

   using loaded_form_type = dovah::loaded_forms::ActorBase;

   auto loaded = stub.load().ptr_cast<loaded_form_type>();
   if (loaded) {
      auto* race = loaded->race.get_form_stub();
      auto  sex  = loaded->get_local_sex();
      if (loaded->template_data.actor && (loaded->template_data.flags & loaded_form_type::template_flag::use_traits)) {
         try {
            auto tmpl_info = dovah::get_all_relevant_template_actors(stub);
            if (auto& src = tmpl_info.templates.traits) {
               race = src->race.get_form_stub();
               sex  = src->get_local_sex();
            }
         } catch (dovah::exceptions::actor_base_template_is_cyclical&) {}
      }
      if (race != this->race)
         return false;
      if (sex != this->sex)
         return false;
   }
   return true;
}

void RacePresetActorFormFilter::setRace(dovah::form_stub* race) {
   if (race == this->race)
      return;
   if (race && race->form_type != dovah::form_type::race)
      return;
   this->race = race;
   this->_refilter_all_forms();
}
void RacePresetActorFormFilter::setSex(dovah::sex sex) {
   if (sex == this->sex)
      return;
   this->sex = sex;
   this->_refilter_all_forms();
}