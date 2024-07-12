#pragma once
#include "../_common.h"
#include <cstdint>
#include <variant>

namespace dovah::loaded_forms::structs {
   //
   // ExtraData for item entries in an inventory component. As of Skyrim, this 
   // data structure can define ownership and health (tempering).
   //
   class container_object_extra_data { // COED subrecord
      protected:
         struct untyped_owner_data {
            int32_t unused = -1;
         };
         struct actor_owner_data {
            form_reference_t global = {}; // unused
         };
         struct faction_owner_data {
            int32_t rank = -1; // Minimum rank an actor must have in the faction to act as though they own the item.
         };

         class ownership_variant {
            friend class container_object_extra_data;
            protected:
               form_reference_t _owner = {};
               std::variant<
                  untyped_owner_data,
                  actor_owner_data,
                  faction_owner_data
               > _condition;

               void _clear_condition_form_refs(loaded_forms::Form& my_containing_form);

            public:
               constexpr form_stub* get_owner() const { return this->_owner.get_form_stub(); }
               constexpr form_stub* get_global() const {
                  const auto* stub = this->get_owner();
                  if (!stub)
                     return nullptr;
                  if (stub->form_type != form_type::actor_base)
                     return nullptr;
                  return std::get<actor_owner_data>(this->_condition).global.get_form_stub();
               }
               constexpr int32_t get_rank() const {
                  const auto* stub = this->get_owner();
                  if (!stub)
                     return -1;
                  if (stub->form_type != form_type::faction)
                     return -1;
                  return std::get<faction_owner_data>(this->_condition).rank;
               }

               void set_owner(loaded_forms::Form& my_containing_form, form_stub*);
               void set_global(loaded_forms::Form& my_containing_form, form_stub*); // Does nothing if the current owner is not an ActorBase.
               void set_rank(loaded_forms::Form& my_containing_form, int32_t); // Does nothing if the current owner is not a faction.
         };

      public:
         float             health = 1.0F; // percentage
         ownership_variant ownership;
         
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_record_writer&, load_order_interfaces::form_save&) const;
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void clone_from(const container_object_extra_data& original, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);
   };
}