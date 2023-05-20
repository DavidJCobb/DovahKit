#pragma once
#include <utility> // std::as_const
#include "./options_union.h"
#include "./id_of.h"

namespace dovahkit::subsystems::worldedit::tools {
   template<tool_or_tool_options T> constexpr bool options_union::is() const noexcept {
      return this->tag == id_of<T>;
   }

   template<tool_with_options_member_type Tool> const typename Tool::options& options_union::as() const noexcept {
      return *((typename Tool::options*)this->data.data());
   }
   template<is_tool_options Options> const Options& options_union::as() const noexcept {
      return *((Options*)this->data.data());
   }

   template<tool_with_options_member_type Tool> typename Tool::options& options_union::as() noexcept {
      return const_cast<typename Tool::options&>(std::as_const(*this).as<Tool>());
   }
   template<is_tool_options Options> Options& options_union::as() noexcept {
      return const_cast<Options&>(std::as_const(*this).as<Options>());
   }

   constexpr bool options_union::operator==(const options_union& other) const {
      if (this->tag != other.tag)
         return false;
      for (const auto& entry : _type_table) {
         if (entry.id == this->tag) {
            return entry.compare(*this, other);
         }
      }
      if (std::is_constant_evaluated()) {
         throw; // No entry in the type table? How did this happen?
      }
      return false;
   }
}