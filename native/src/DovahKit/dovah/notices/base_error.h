#pragma once
#include <type_traits>

namespace dovah::notices {
   class base_error {
      public:
         virtual ~base_error() {}
         virtual base_error* clone() const = 0;
   };
}