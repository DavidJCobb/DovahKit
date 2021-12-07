#pragma once
#include <QString>
#include "BoundInput.h"
#include "tools/_options.h"

namespace DK3D {
   namespace tools {
      class base;
   }

   struct Binding {
      QString name;
      BoundInput input;
      const tools::base*  function = nullptr;
      tools::option_union params;

      Binding() {}

      template<typename T> requires tools::tool_has_options_member_type<T>
      Binding(const QString& n, const BoundInput& bi, T* f, const typename T::options& o) : name(n), input(bi), function(f), params(o) {}

      template<typename T> requires tools::tool_lacks_options_member_type<T>
      Binding(const QString& n, const BoundInput& bi, T* f) : name(n), input(bi), function(f) {}
   };
}