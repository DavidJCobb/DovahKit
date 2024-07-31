#include "./actor_creature_sounds.h"
#include "../_common_cpp.h"

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
         dst.reserve(this->_own_sounds.size());
         for (auto& item : this->_own_sounds) {
            if (!item.sound)
               continue;
            auto& dst_item = dst.emplace_back();
            dst_item.chance = item.chance;
            dst_item.type   = item.type;
            dst_item.sound  = item.sound.get_form_stub();
         }
      }
      return dst;
   }

   void actor_creature_sounds::set_inherits_from(loaded_forms::Form& my_containing_form, dovah::form_stub* actor) {
      this->_inherit_from.set(my_containing_form, actor);
      if (actor) {
         for (auto& item : this->_own_sounds)
            item.sound.set(my_containing_form, nullptr);
         this->_own_sounds.clear();
      }
   }

   void actor_creature_sounds::add_sound(loaded_forms::Form& my_containing_form, creature_sound_type type, dovah::form_stub* sound, uint8_t chance) {
      if (this->_inherit_from)
         return;
      auto& item = this->_own_sounds.emplace_back();
      item.type   = type;
      item.chance = chance;
      item.sound.set(my_containing_form, sound);
   }
   void actor_creature_sounds::replace_sounds(loaded_forms::Form& my_containing_form, const std::vector<entry>& src) {
      if (this->_inherit_from)
         return;
      auto& dst_list = this->_own_sounds;
      for (auto& dst_item : dst_list)
         dst_item.sound.set(my_containing_form, nullptr);

      size_t size = src.size();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_item = src[i];
         auto& dst_item = dst_list[i];
         dst_item.type = src_item.type;
         dst_item.chance = src_item.chance;
         dst_item.sound.set(my_containing_form, src_item.sound);
      }
   }
   size_t actor_creature_sounds::sound_count() const {
      if (this->_inherit_from)
         return 0;
      return this->_own_sounds.size();
   }

   #pragma region Form utils
   void actor_creature_sounds::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case subrecord_signature_inherit:
            this->_own_sounds.clear();

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
               auto& e = this->_loader_state.pending_entry.emplace();
               subrecord.read(e.type);
            }
            break;
         case subrecord_signature_sound_form:
            {
               auto& pending_opt = this->_loader_state.pending_entry;
               if (!pending_opt.has_value())
                  break;
               if (subrecord.get_containing_record().peek_next_subrecord_type() != subrecord_signature_sound_chance) {
                  pending_opt = {};
                  break;
               }

               this->_inherit_from.unmanaged_set(nullptr);

               form_reference_t form_id;
               if (subrecord.read(form_id)) {
                  intfc.warn_if_ref_is_wrong_type(form_id, form_type::sound_descriptor, subrecord.signature());
                  pending_opt.value().sound = form_id.get_form_stub();
               }
            }
            break;
         case subrecord_signature_sound_chance:
            {
               auto& pending_opt = this->_loader_state.pending_entry;
               if (!pending_opt.has_value())
                  break;
               auto& pending = pending_opt.value();
               subrecord.read(pending.chance);

               auto& dst = this->_own_sounds.emplace_back();
               dst.chance = pending.chance;
               dst.type   = pending.type;
               dst.sound.unmanaged_set(pending.sound);

               pending_opt = {};
            }
            break;
      }
   }
   void actor_creature_sounds::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) const {
      if (this->_inherit_from)
         record.write_formID_subrecord(subrecord_signature_inherit, this->_inherit_from);
      else {
         for (auto& item : this->_own_sounds) {
            auto& subrecord_a = record.open_next_subrecord(subrecord_signature_sound_start);
            subrecord_a.write(item.type);
            subrecord_a.close();
            record.write_formID_subrecord(subrecord_signature_sound_form, item.sound.get_form_stub(), false);
            auto& subrecord_c = record.open_next_subrecord(subrecord_signature_sound_chance);
            subrecord_c.write(item.chance);
            subrecord_c.close();
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
            uib.pending_entry_is_valid = true;
            break;
         case subrecord_signature_sound_form:
            if (!uib.pending_entry_is_valid)
               break;
            {
               if (subrecord.get_containing_record().peek_next_subrecord_type() != subrecord_signature_sound_chance) {
                  uib.pending_entry_is_valid = false;
                  break;
               }
               uib.inherit_from = {};

               form_id_t form_id;
               if (subrecord.read(form_id)) {
                  uib.sounds.push_back(form_id);
               }
            }
            break;
         case subrecord_signature_sound_chance:
            uib.pending_entry_is_valid = false;
            break;
      }
   }
   void actor_creature_sounds::clone_from(const actor_creature_sounds& original, loaded_forms::Form& my_containing_form) noexcept {
      this->clear(my_containing_form);
      if (original._inherit_from) {
         this->_inherit_from.set(my_containing_form, original._inherit_from);
      } else {
         size_t size = original._own_sounds.size();
         this->_own_sounds.reserve(size);
         for (auto& src_item : original._own_sounds) {
            auto& dst_item = this->_own_sounds.emplace_back();
            dst_item.chance = src_item.chance;
            dst_item.type   = src_item.type;
            dst_item.sound.set(my_containing_form, src_item.sound);
         }
      }
   }
   void actor_creature_sounds::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      this->_inherit_from.clear_if(my_containing_form, target);
      {
         bool any_lost = false;
         for (auto& item : this->_own_sounds) {
            if (item.sound == &target) {
               any_lost = true;
               item.sound.set(my_containing_form, nullptr);
            }
         }
         if (any_lost) {
            std::erase_if(this->_own_sounds, [](const _stored_entry& item) -> bool {
               return item.sound.get_form_stub() == nullptr;
            });
         }
      }
   }
   void actor_creature_sounds::clear(loaded_forms::Form& my_containing_form) {
      for (auto& item : this->_own_sounds)
         item.sound.set(my_containing_form, nullptr);
      this->_own_sounds.clear();
      this->_inherit_from.set(my_containing_form, nullptr);
   }
   #pragma endregion
}