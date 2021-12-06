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

      template<typename T> requires requires { typename T::options; }
      Binding(const BoundInput& bi, T* f, const typename T::options& o) : input(bi), function(f), params(o) {}
   };
}