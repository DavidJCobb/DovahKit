#include "./actor_base.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/ActorBase.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::ActorBase;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void actor_base::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'ACBS':
            {
               decltype(loaded_form_type::actor_flags) flags = 0;
               if (subrecord.read(flags)) {
                  this->female     = flags & loaded_form_type::actor_flag::female;
                  this->summonable = flags & loaded_form_type::actor_flag::summonable;
                  this->unique     = flags & loaded_form_type::actor_flag::unique;
               }
            }
            break;
         case 'VTCK':
            this->_read_form_stub_from_subrecord(this->voicetype, subrecord, dovah::form_type::voicetype);
            break;
      }
   }
   bool actor_base::sever_outbound_references_to(const dovah::form_stub* target) {
      if (this->voicetype == target) {
         this->voicetype = nullptr;
         return true;
      }
      return false;
   }
   bool actor_base::update(const loaded_form_type& src) {
      auto flags   = src.actor_flags;
      bool changed = false;
      auto _update = [&changed, flags](bool& dst, loaded_form_type::actor_flag::type flag) {
         bool current = flags & flag;
         if (dst != current) {
            dst     = current;
            changed = true;
         }
      };
      _update(this->female,     loaded_form_type::actor_flag::female);
      _update(this->summonable, loaded_form_type::actor_flag::summonable);
      _update(this->unique,     loaded_form_type::actor_flag::unique);

      this->_update_form_stub_from_loaded(this->voicetype, src.voicetype, dovah::form_type::voicetype);

      return changed;
   }
}