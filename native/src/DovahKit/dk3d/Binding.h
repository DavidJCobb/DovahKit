#pragma once
#include "BoundInput.h"
#include "editor_functions/_options.h"

namespace DK3D {
   namespace editor_functions {
      class base;
   }

   struct Binding {
      BoundInput input;
      editor_functions::base*        function = nullptr;
      editor_functions::option_union params;

      Binding() {}

      template<typename T> requires requires { typename T::options; }
      Binding(const BoundInput& bi, T* f, const typename T::options& o) : input(bi), function(f), params(o) {}
   };
}