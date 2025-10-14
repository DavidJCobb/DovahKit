#pragma once
#include "./_base_node.h"

namespace IdleAnimationFormsModel_impl {
   template<node_type Enum>
   const utils::node_type_from_enum<Enum>* node::as() const noexcept {
      if (this->type != Enum)
         return nullptr;
      return (utils::node_type_from_enum<Enum>*)this;
   }

   template<node_type Enum>
   utils::node_type_from_enum<Enum>* node::as() noexcept {
      return const_cast<utils::node_type_from_enum<Enum>*>(std::as_const(*this).as<Enum>());
   }
}