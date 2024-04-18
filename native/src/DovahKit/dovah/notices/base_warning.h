#pragma once
#include <type_traits>

namespace dovah::notices {
   class base_warning {
      public:
         virtual ~base_warning() {}
         virtual base_warning* clone() const = 0;
   };
}