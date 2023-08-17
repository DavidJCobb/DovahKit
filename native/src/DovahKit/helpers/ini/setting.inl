#pragma once
#include "./setting.h"
#include <optional>
#include <stdexcept>

namespace cobb::ini {
   template<typename T>
   constexpr setting::setting(category& cat, const setting_definition<T>& dfn) : owner(cat), name(dfn.name) {
      auto& info = this->info.emplace<info_type<T>>();
      info.values.current = info.values.initial = dfn.initial_value;
      info.constraints = dfn.constraints;
      if (std::is_constant_evaluated()) {
         if (!info.constraints.allows(info.values.initial)) {
            throw std::logic_error("INI setting definition's initial value does not meet its own constraints");
         }
      }

      this->owner._on_setting_instantiated({}, *this);
   }

   template<auto Definition> requires (setting_definition_type<std::decay_t<decltype(Definition)>> && (prefer_static_assertions || Definition.is_valid()))
   /*static*/ constexpr setting setting::define(category& c) {
      using parameter_type = std::decay_t<decltype(Definition)>;

      static_assert(Definition.name != nullptr, "The setting definition must have a name.");
      static_assert(Definition.name[0] == value_types::template value_of<typename parameter_type::value_type>, "The setting definition's type must match its Hungarian-notation prefix.");
      static_assert(Definition.constraints.allows(Definition.initial_value), "The setting definition's initial value must satisfy any constraints present in the definition.");

      return setting(c, Definition);
   }

   template<typename T>
   constexpr bool setting::is_of_type() const noexcept {
      return value_types::for_each_pair_until_true([this]<typename Key, char Value>() {
         return Value == this->name[0];
      });
   }

   template<typename T> requires value_types::has_key<T>
   constexpr const value_constraint_info<T>& setting::get_constraints() const {
      if (!this->is_of_type<T>())
         throw std::logic_error("incorrect type specified for INI setting");
      return std::get<info_type<T>>(this->info).constraints;
   }

   template<typename T> requires value_types::has_key<T>
   constexpr T setting::get_initial_value() const {
      if (!this->is_of_type<T>())
         throw std::logic_error("incorrect type specified for INI setting");
      return std::get<info_type<T>>(this->info).values.initial;
   }

   template<typename T> requires value_types::has_key<T>
   constexpr T setting::get_current_value() const {
      if (!this->is_of_type<T>())
         throw std::logic_error("incorrect type specified for INI setting");
      return std::get<info_type<T>>(this->info).values.current;
   }

   constexpr value_variant setting::get_current_value_variant() const {
      // Use std::optional as a workaround for `value_variant` not being default-constructible 
      // (because it does not allow std::monostate, because a setting cannot be "empty").
      std::optional<value_variant> out;
      value_types::for_each_pair_until_true([&out, this]<typename Key, char Value>() {
         if (Value == this->name[0]) {
            out = this->get_current_value<Key>();
            return true;
         }
         return false;
      });
      return out.value();
   }

   template<typename T> requires value_types::has_key<T>
   constexpr void setting::set_current_value(const T& v) const {
      if (!this->is_of_type<T>())
         throw std::logic_error("incorrect type specified for INI setting");
      auto& info = std::get<info_type<T>>(this->info);
      if (!info.constraints.allows(v))
         throw std::domain_error("specified value does not meet this INI setting's constraints");
      if (info.values.current == v)
         return;
      value_union prior = info.values.current;
      info.values.current = v;
      this->owner._on_setting_changed({}, *this, prior, v);
   }

   // The dummy bool used here is: https://devblogs.microsoft.com/oldnewthing/20221202-00/?p=107532
   // "C++ template parlor tricks: Using a type before it is defined" by Raymond Chen (December 2, 2022)
   template<bool Dummy>
   constexpr void setting::set_current_value_variant(const value_variant& v) {
      value_types::for_each_pair_until_true([this, &v]<typename Key, char Value>() {
         if (Value == this->name[0]) {
            if (!std::holds_alternative<Key>(v))
               throw;

            Key& target = std::get<info_type<Key>>(this->info).values.current;
            Key  after  = std::get<Key>(v);
            if (target != after) {
               Key prior = target;
               target = after;
               this->owner._on_setting_changed({}, *this, prior, after);
            }
            return true;
         }
         return false;
      });
   }
}
