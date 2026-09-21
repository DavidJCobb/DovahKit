#include "./actor_creature_sounds.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/actor_creature_sounds/invalid_sound_type.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::actor_creature_sounds;
   }
}

namespace dovah::loaded_forms::structs {
   void actor_creature_sounds::use_info_builder::done() {
      if (this->inherit_from) {
         assert(this->sounds.empty());
         this->owner.add_outbound_reference(this->inherit_from);
      } else {
         for (auto id : this->sounds)
            this->owner.add_outbound_reference(id);
      }
   }

   //

   form_stub* actor_creature_sounds::inherits_from() const {
      return this->_inherit_from.get_form_stub();
   }
   std::vector<actor_creature_sounds::entry> actor_creature_sounds::sounds() const {
      std::vector<actor_creature_sounds::entry> dst;
      if (!this->_inherit_from) {
         for (size_t i = 0; i < num_creature_sound_types; ++i) {
            auto& list = this->_own_sounds[i];
            for (auto& item : list) {
               if (!item.sound)
                  continue;
               auto& dst_item = dst.emplace_back();
               dst_item.chance = item.chance;
               dst_item.type   = (creature_sound_type)i;
               dst_item.sound  = item.sound.get_form_stub();
            }
         }
      }
      return dst;
   }

   void actor_creature_sounds::set_inherits_from(loaded_forms::Form& my_containing_form, dovah::form_stub* actor) {
      this->_inherit_from.set(my_containing_form, actor);
      if (actor) {
         for (auto& list : this->_own_sounds) {
            for (auto& item : list)
               item.sound.set(my_containing_form, nullptr);
            list.clear();
         }
      }
   }

   void actor_creature_sounds::add_sound(loaded_forms::Form& my_containing_form, creature_sound_type type, dovah::form_stub* sound, uint8_t chance) {
      if (this->_inherit_from)
         return;
      assert((size_t)type < num_creature_sound_types);
      auto& item = this->_own_sounds[(size_t)type].emplace_back();
      item.chance = chance;
      item.sound.set(my_containing_form, sound);
   }
   void actor_creature_sounds::replace_sounds(loaded_forms::Form& my_containing_form, const std::vector<entry>& src) {
      if (this->_inherit_from)
         return;
      for (auto& list : this->_own_sounds) {
         for (auto& item : list)
            item.sound.set(my_containing_form, nullptr);
         list.clear();
      }
      for (auto& src_item : src) {
         assert((size_t)src_item.type < num_creature_sound_types);
         auto& dst_item = this->_own_sounds[(size_t)src_item.type].emplace_back();
         dst_item.chance = src_item.chance;
         dst_item.sound.set(my_containing_form, src_item.sound);
      }
   }
   size_t actor_creature_sounds::sound_count() const {
      if (this->_inherit_from)
         return 0;
      size_t size = 0;
      for (auto& list : this->_own_sounds)
         size += list.size();
      return size;
   }

   #pragma region Form utils
   void actor_creature_sounds::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case subrecord_signature_inherit:
            for(auto& list : this->_own_sounds)
               list.clear();
            this->_loader_state.pending_type = {};
            this->_loader_state.pending_form = {};

            if (subrecord.read(this->_inherit_from)) {
               intfc.warn_if_ref_is_wrong_type(this->_inherit_from, form_type::actor_base, subrecord.signature());
            }
            break;

         //
         // The ActorBase loader is structured as follows:
         // 
         //  - The CSDT subrecord's value is read into a local variable, but nothing is done 
         //    with it at first.
         // 
         //  - When CSDI is read, the game clears any existing inherit-actor pointer, and then 
         //    opens the next subrecord. If that next subrecord is CSDC, then a new creature 
         //    sound is added; otherwise nothing is done.
         // 
         //     - Commonly, when one subrecord triggers opening of the next subrecord, that 
         //       next subrecord will be blindly read or skipped if it's not what the game was 
         //       expecting. However, the ActorBase loader is specifically structured (maybe 
         //       with gotos?) so that it doesn't do this.
         //

         case subrecord_signature_sound_start:
            {
               creature_sound_type v;
               if (subrecord.read(v)) {
                  if ((size_t)v < num_creature_sound_types) {
                     this->_loader_state.pending_type = v;
                  } else {
                     specific_load_warnings::invalid_sound_type notice(
                        intfc.target_stub,
                        (uint32_t)v
                     );
                     intfc.log_load_warning(notice);
                     //
                     this->_loader_state.pending_type = {};
                  }
               } else {
                  this->_loader_state.pending_type = {};
               }
               this->_loader_state.pending_form = {};
            }
            break;
         case subrecord_signature_sound_form:
            {
               if (!this->_loader_state.pending_type.has_value())
                  break;
               auto& dst = this->_loader_state.pending_form;
               if (subrecord.get_containing_record().peek_next_subrecord_type() != subrecord_signature_sound_chance) {
                  dst = {};
                  break;
               }

               this->_inherit_from.unmanaged_set(nullptr);

               form_reference_t form_id;
               if (subrecord.read(form_id)) {
                  intfc.warn_if_ref_is_wrong_type(form_id, form_type::sound_descriptor, subrecord.signature());
                  dst = form_id.get_form_stub();
               }
            }
            break;
         case subrecord_signature_sound_chance:
            {
               if (!this->_loader_state.pending_type.has_value())
                  break;
               if (!this->_loader_state.pending_form.has_value())
                  break;
               auto* form = this->_loader_state.pending_form.value().get_form_stub();

               auto& dst = this->_own_sounds[(size_t)this->_loader_state.pending_type.value()].emplace_back();
               subrecord.read(dst.chance);
               dst.sound.unmanaged_set(form);

               this->_loader_state.pending_form = {};
            }
            break;
      }
   }
   void actor_creature_sounds::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) const {
      if (this->_inherit_from)
         record.write_formID_subrecord(subrecord_signature_inherit, this->_inherit_from);
      else {
         for (size_t i = 0; i < num_creature_sound_types; ++i) {
            auto& list = this->_own_sounds[i];
            if (list.empty())
               continue;
            auto& subrecord_a = record.open_next_subrecord(subrecord_signature_sound_start);
            subrecord_a.write((creature_sound_type)i);
            subrecord_a.close();
            for (auto& item : list) {
               record.write_formID_subrecord(subrecord_signature_sound_form, item.sound.get_form_stub(), false);
               auto& subrecord_c = record.open_next_subrecord(subrecord_signature_sound_chance);
               subrecord_c.write(item.chance);
               subrecord_c.close();
            }
         }
      }
      return;
   }
   /*static*/ void actor_creature_sounds::generate_use_info(tes_subrecord_reader& subrecord, use_info_builder& uib) {
      switch (subrecord.signature()) {
         case subrecord_signature_inherit:
            uib.sounds.clear();
            subrecord.read(uib.inherit_from);
            break;

         case subrecord_signature_sound_start:
            {
               std::underlying_type_t<creature_sound_type> type;
               if (subrecord.read(type)) {
                  if (type >= num_creature_sound_types) {
                     uib.seen_type      = false;
                     uib.seen_form      = false;
                     uib.last_seen_form = {};
                     break;
                  }
               }
            }
            uib.seen_type      = true;
            uib.seen_form      = false;
            uib.last_seen_form = {};
            break;
         case subrecord_signature_sound_form:
            if (!uib.seen_type)
               break;
            if (subrecord.get_containing_record().peek_next_subrecord_type() != subrecord_signature_sound_chance) {
               uib.seen_form = false;
               break;
            }
            uib.inherit_from = {};
            uib.seen_form    = true;
            subrecord.read(uib.last_seen_form);
            break;
         case subrecord_signature_sound_chance:
            if (uib.seen_type && uib.seen_form) {
               if (uib.last_seen_form)
                  uib.sounds.push_back(uib.last_seen_form);
               uib.last_seen_form = {};
            }
            uib.seen_form = false;
            break;
      }
   }
   void actor_creature_sounds::clone_from(const actor_creature_sounds& original, loaded_forms::Form& my_containing_form) noexcept {
      this->clear(my_containing_form);
      if (original._inherit_from) {
         this->_inherit_from.set(my_containing_form, original._inherit_from);
      } else {
         for (size_t i = 0; i < this->_own_sounds.size(); ++i) {
            auto& src_list = original._own_sounds[i];
            auto& dst_list = this->_own_sounds[i];
            dst_list.reserve(src_list.size());
            for (auto& src_item : src_list) {
               auto& dst_item = dst_list.emplace_back();
               dst_item.chance = src_item.chance;
               dst_item.sound.set(my_containing_form, src_item.sound);
            }
         }
      }
   }
   void actor_creature_sounds::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      this->_inherit_from.clear_if(my_containing_form, target);
      for (auto& list : this->_own_sounds) {
         bool any_lost = false;
         for (auto& item : list) {
            if (item.sound == &target) {
               any_lost = true;
               item.sound.set(my_containing_form, nullptr);
            }
         }
         if (any_lost) {
            std::erase_if(list, [](const _stored_entry& item) -> bool {
               return item.sound.get_form_stub() == nullptr;
            });
         }
      }
   }
   void actor_creature_sounds::clear(loaded_forms::Form& my_containing_form) {
      for (auto& list : this->_own_sounds) {
         for (auto& item : list)
            item.sound.set(my_containing_form, nullptr);
         list.clear();
      }
      this->_inherit_from.set(my_containing_form, nullptr);
   }
   #pragma endregion
}