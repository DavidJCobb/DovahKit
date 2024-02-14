#pragma once
#include "./known_script.h"
#include "dovah/data/papyrus/helpers/name_equals.h"

namespace dovahkit::subsystems::papyrus {
   constexpr bool known_script::name_matches(std::string_view n) const {
      return dovah::papyrus::helpers::name_equals(this->name, n);
   }
   constexpr const known_script* known_script::superclass() const {
      if (this->info.loose.has_value())
         return this->info.loose.value().extends.target;
      if (this->info.packed.has_value())
         return this->info.packed.value().extends.target;
      return nullptr;
   }
   constexpr known_script* known_script::superclass() {
      return const_cast<known_script*>(std::as_const(*this).superclass());
   }

   constexpr std::optional<dovah::form_type_t> known_script::underlying_type() const {
      if (this->inheritance.root_class)
         return this->inheritance.root_class->underlying_type();

      if (this->info.loose.has_value())
         return this->info.loose.value().extends.underlying_type;
      if (this->info.packed.has_value())
         return this->info.packed.value().extends.underlying_type;

      return {};
   }
   constexpr bool known_script::is_attachable_to(dovah::form_type_t desired) const {
      auto under_opt = this->underlying_type();
      if (!under_opt.has_value())
         return false;

      auto underlying = under_opt.value();

      // Handle scripts not attachable to forms.
      switch (underlying) {
         case dovah::form_type::alias:
         case dovah::form_type::location_alias:
         case dovah::form_type::reference_alias:
            if (desired == dovah::form_type::alias)
               return true;
            return underlying == desired;

         case dovah::form_type::active_magic_effect:
            return desired == dovah::form_type::magic_effect;
      }

      // Early-out if we just want to know if the script is attachable to any form.
      if (desired == dovah::form_type::none) {
         return true;
      }

      // ObjectReference scripts are the only ones that can be meaningfully attached to base forms.
      // Trying to attach a script that subclasses some base form type X, to an X, will fail in-game.
      if (dovah::form_type_info::form_type_is_base_form(desired)) {
         if (desired == dovah::form_type::actor_base && underlying == dovah::form_type::actor) {
            return true;
         }
         return underlying == dovah::form_type::reference;
      }

      if (underlying == desired)
         return true;

      // Check if `desired` is an ancestor class of the one we attach to .
      // (e.g. if we extend Furniture and `desired` is Activator)
      auto* info = &dovah::form_type_info::lookup(underlying);
      while (info && info->parent_type) {
         if (desired == info->parent_type)
            return true;
         info = &dovah::form_type_info::lookup(info->parent_type);
      }

      return false;
   }
   constexpr bool known_script::is_of_type(std::string_view desired) const {
      if (this->name_matches(desired))
         return true;

      const auto* sc = this;
      while (sc = sc->superclass())
         if (sc->name_matches(desired))
            return true;
   
      return false;
   }

   //

   constexpr bool known_script::is_unreferenced() const {
      if (!this->is_unreferenced_except_by_loose())
         return false;

      if (!this->inheritance.potential_subclasses.loose.empty())
         return false;

      return true;
   }
   constexpr bool known_script::is_unreferenced_except_by_loose() const {
      if (this->refcount > 0)
         return false;

      if (!this->inheritance.potential_subclasses.packed.empty())
         return false;

      return true;
   }
   
   template<typename Functor> requires (std::is_invocable_v<Functor, const known_script&>)
   constexpr void known_script::for_each_child_class(Functor&& functor) const {
      auto& list_set = this->inheritance.potential_subclasses;
      for (const auto* child : list_set.loose) {
         if (child->inheritance.root_class == this->inheritance.root_class) // early-out superclass check
            continue;
         if (child->superclass() != this)
            continue;
         functor(*child);
      }
      for (const auto* child : list_set.packed) {
         if (child->inheritance.root_class == this->inheritance.root_class) // early-out superclass check
            continue;
         if (child->superclass() != this)
            continue;
         functor(*child);
      }
   }
   //
   template<typename Functor> requires (std::is_invocable_v<Functor, known_script&>)
   constexpr void known_script::for_each_child_class(Functor&& functor) {
      auto& list_set = this->inheritance.potential_subclasses;
      for (auto* child : list_set.loose) {
         if (child->inheritance.root_class == this->inheritance.root_class) // early-out superclass check
            continue;
         if (child->superclass() != this)
            continue;
         functor(*child);
      }
      for (auto* child : list_set.packed) {
         if (child->inheritance.root_class == this->inheritance.root_class) // early-out superclass check
            continue;
         if (child->superclass() != this)
            continue;
         functor(*child);
      }
   }

   template<typename Functor> requires (std::is_invocable_v<Functor, const known_script&>)
   constexpr void known_script::for_each_descendant_class(Functor&& functor) const {
      for_each_child_class([&functor](const known_script& child) {
         functor(child);
         child.for_each_descendant_class(functor);
      });
   }
   //
   template<typename Functor> requires (std::is_invocable_v<Functor, known_script&>)
   constexpr void known_script::for_each_descendant_class(Functor&& functor) {
      for_each_child_class([&functor](known_script& child) {
         functor(child);
         child.for_each_descendant_class(functor);
      });
   }
}