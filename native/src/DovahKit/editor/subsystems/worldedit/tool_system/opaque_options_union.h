#pragma once
#include "./tool_id.h"

namespace dovahkit::subsystems::worldedit::tools {
   class options_union;
}

namespace dovahkit::subsystems::worldedit::tools {
   class opaque_options_union {
      protected:
         tool_id tag = id_of_none;

         constexpr opaque_options_union(tool_id t) : tag(t) {}

      public:
         constexpr tool_id id() const noexcept { return this->tag; }

         constexpr bool empty() const noexcept { return this->id() == id_of_none; }

         template<typename T> const T& as() const { return ((const options_union*)this)->template as<T>(); }
         template<typename T> T& as() { return ((options_union*)this)->template as<T>(); }
   };
}