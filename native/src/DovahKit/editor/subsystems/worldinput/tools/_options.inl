#pragma once
#include "_options.h"

namespace dovahkit::subsystems::worldinput::tools {
   template<is_tool_or_options T> bool option_union::is() const noexcept {
      if (is_tool<T>) {
         return this->tag == id_of_tool<T>();
      } else {
         return this->tag == id_of_tool_options<T>();
      }
   }
   template<is_tool_or_options T> /*static*/ bool option_union::is(const option_union_base& b) {
      return ((const option_union*)&b)->is<T>();
   }

   template<tools::tool_has_options_member_type Tool> const typename Tool::options* option_union::as() const noexcept {
      if (!is<Tool>())
         return nullptr;
      return (typename Tool::options*)this->data.data();
   }
   template<is_tool_options Options> const Options* option_union::as() const noexcept {
      if (!is<Options>())
         return nullptr;
      return (Options*)this->data.data();
   }

   template<tools::tool_has_options_member_type Tool> typename Tool::options* option_union::as() noexcept {
      return const_cast<typename Tool::options*>(_as_const()->as<Tool>());
   }
   template<is_tool_options Options> Options* option_union::as() noexcept {
      return const_cast<Options*>(_as_const()->as<Options>());
   }

   template<tools::tool_has_options_member_type Tool> /*static*/ const typename Tool::options* option_union::as(const option_union_base& b) {
      return ((const option_union*)&b)->as<Tool>();
   }
   template<is_tool_options Options> /*static*/ const Options* option_union::as(const option_union_base& b) {
      return ((const option_union*)&b)->as<Options>();
   }

   template<tools::tool_has_options_member_type Tool> /*static*/ typename Tool::options* option_union::as(option_union_base& b) {
      return ((option_union*)&b)->as<Tool>();
   }
   template<is_tool_options Options> /*static*/ Options* option_union::as(option_union_base& b) {
      return ((option_union*)&b)->as<Options>();
   }
}