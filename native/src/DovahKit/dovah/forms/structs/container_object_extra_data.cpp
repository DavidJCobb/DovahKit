#include "./container_object_extra_data.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/container/item_has_bad_owner_form_type.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::container;
   }
}

namespace dovah::loaded_forms::structs {
   void container_object_extra_data::ownership_variant::_clear_condition_form_refs(loaded_forms::Form& my_containing_form) {
      if (auto* casted = std::get_if<actor_owner_data>(&this->_condition)) {
         casted->global.set(my_containing_form, nullptr);
      }
   }

   void container_object_extra_data::ownership_variant::set_owner(loaded_forms::Form& my_containing_form, form_stub* owner) {
      if (this->get_owner() == owner)
         return;
      this->_owner.set(my_containing_form, owner);
      switch (owner->form_type) {
         case form_type::actor_base:
            if (!std::holds_alternative<actor_owner_data>(this->_condition)) {
               this->_clear_condition_form_refs(my_containing_form);
               this->_condition = actor_owner_data{};
            }
            break;
         case form_type::faction:
            if (!std::holds_alternative<faction_owner_data>(this->_condition)) {
               this->_clear_condition_form_refs(my_containing_form);
               this->_condition = faction_owner_data{};
            }
            break;
         default:
            if (!std::holds_alternative<untyped_owner_data>(this->_condition)) {
               this->_clear_condition_form_refs(my_containing_form);
               this->_condition = untyped_owner_data{};
            }
            break;
      }
   }
   void container_object_extra_data::ownership_variant::set_global(loaded_forms::Form& my_containing_form, form_stub* global) {
      if (auto* casted = std::get_if<actor_owner_data>(&this->_condition))
         casted->global.set(my_containing_form, global);
   }
   void container_object_extra_data::ownership_variant::set_rank(loaded_forms::Form& my_containing_form, int32_t rank) {
      if (auto* casted = std::get_if<faction_owner_data>(&this->_condition))
         casted->rank = rank;
   }

   void container_object_extra_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (!subrecord.read(this->ownership._owner)) {
         return;
      }
      {
         auto* owner_stub = subrecord.lookup_form_by_id(this->ownership._owner);
         intfc.warn_if_ref_is_wrong_type(this->ownership._owner, std::array{ form_type::actor_base, form_type::faction }, subrecord.signature());
         if (owner_stub) {
            switch (owner_stub->form_type) {
               case form_type::actor_base:
                  {
                     this->ownership._condition = actor_owner_data{};
                     auto& dst = std::get<actor_owner_data>(this->ownership._condition);
                     subrecord.read(dst.global);
                     intfc.warn_if_ref_is_wrong_type(dst.global, form_type::global, subrecord.signature());
                  }
                  break;
               case form_type::faction:
                  {
                     this->ownership._condition = faction_owner_data{};
                     auto& dst = std::get<faction_owner_data>(this->ownership._condition);
                     subrecord.read(dst.rank);
                  }
                  break;
               default:
                  {
                     specific_load_warnings::item_has_bad_owner_form_type notice(
                        intfc.target_stub,
                        *owner_stub
                     );
                     intfc.log_load_warning(notice);

                     this->ownership._condition = untyped_owner_data{};
                     auto& dst = std::get<untyped_owner_data>(this->ownership._condition);
                     subrecord.read(dst.unused);
                  }
                  break;
            }
         }
      }
      subrecord.read(this->health);
   }
   void container_object_extra_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) const {
      if (this->ownership._owner == nullptr && this->health == 1.0F)
         return;
      
      auto& COED = record.open_next_subrecord('COED');
      {
         COED.write(this->ownership._owner);
         auto& src_variant = this->ownership._condition;
         if (auto* stub = this->ownership.get_owner()) {
            if (stub->form_type == form_type::actor_base) {
               assert(std::holds_alternative<actor_owner_data>(src_variant));
               const auto& src = std::get<actor_owner_data>(src_variant);
               COED.write(src.global);
            } else if (stub->form_type == form_type::faction) {
               assert(std::holds_alternative<faction_owner_data>(src_variant));
               const auto& src = std::get<faction_owner_data>(src_variant);
               COED.write(src.rank);
            } else {
               assert(std::holds_alternative<untyped_owner_data>(src_variant));
               const auto& src = std::get<untyped_owner_data>(src_variant);
               COED.write(src.unused);
            }
         } else {
            assert(std::holds_alternative<untyped_owner_data>(src_variant));
            const auto& src = std::get<untyped_owner_data>(src_variant);
            COED.write(src.unused);
         }
      }
      COED.write(this->health);
      COED.close();
   }
   /*static*/ void container_object_extra_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      //
      // There's a formID followed by an integer/formID union whose type depends on the form 
      // type of the formID preceding it. Fortunately, by the time we're building Use Info, 
      // we've already identified all forms and their types.
      //
      form_id_t formID;
      if (subrecord.read(formID)) {
         uib.add_outbound_reference(formID);
         //
         if (formID) {
            const auto* owner_stub = subrecord.lookup_form_by_id(formID);
            if (owner_stub && owner_stub->form_type == form_type::actor_base) {
               if (subrecord.read(formID)) // owner GLOB
                  uib.add_outbound_reference(formID);
            }
         }
      }
   }
   void container_object_extra_data::clone_from(const container_object_extra_data& original, loaded_forms::Form& my_containing_form) noexcept {
      {
         auto* owner = original.ownership.get_owner();
         this->ownership.set_owner(my_containing_form, owner);
         if (owner) {
            switch (owner->form_type) {
               case form_type::actor_base:
                  this->ownership.set_global(my_containing_form, original.ownership.get_global());
                  break;
               case form_type::faction:
                  this->ownership.set_rank(my_containing_form, original.ownership.get_rank());
                  break;
            }
         }
         if (std::holds_alternative<untyped_owner_data>(this->ownership._condition)) {
            auto& src = std::get<untyped_owner_data>(original.ownership._condition);
            auto& dst = std::get<untyped_owner_data>(this->ownership._condition);
            dst = src;
         }
      }
      this->health = original.health;
   }
   void container_object_extra_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      if (this->ownership.get_owner() == &target) {
         this->ownership.set_owner(my_containing_form, nullptr);
      } else {
         if (auto* casted = std::get_if<actor_owner_data>(&this->ownership._condition)) {
            if (casted->global == &target)
               casted->global.set(my_containing_form, nullptr);
         }
      }
   }
   void container_object_extra_data::clear(loaded_forms::Form& my_containing_form) {
      this->ownership.set_owner(my_containing_form, nullptr);
      std::get<untyped_owner_data>(this->ownership._condition).unused = 0;

      this->health = 1.0F;
   }
}