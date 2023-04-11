#pragma once

#include <string>
#include <type_traits>
#include "../debugging.h"

namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results {

      protected:
         struct dummy {};
      public:
         std::conditional_t<enable_initial_testing, std::string, dummy> data;

      public:
         // TODO

         void merge(const combined_tool_results& other) {
            if constexpr (enable_initial_testing) {
               this->data += '\n';
               this->data += other.data;
            }
         }
         void scale(double) {}
   };
}