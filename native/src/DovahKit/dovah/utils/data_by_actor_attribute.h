#pragma once
#include <array>
#include <type_traits> // std::is_within_lifetime and __cpp_lib_is_within_lifetime

namespace dovah {
   template<typename ValueType>
   union data_by_actor_attribute {
      public:
         using value_type = ValueType;

         #pragma region Union boilerplate
         constexpr data_by_actor_attribute() {}
         //
         // NOTE: Omitting the const-other constructor causes an internal compiler error 
         //       on MSVC as of 8/14/2024.
         //
         #if __cpp_lib_is_within_lifetime
            constexpr data_by_actor_attribute(const data_by_actor_attribute& other) {
               if (std::is_within_lifetime(&other.h)) {
                  this->list = { other.h, other.m, other.s };
               } else if (std::is_within_lifetime(&other.health)) {
                  this->list = { other.health, other.magicka, other.stamina };
               } else {
                  this->list = other.list;
               }
            }
         #else
            data_by_actor_attribute(const data_by_actor_attribute& other) : list(other.list) {}
         #endif

         constexpr ~data_by_actor_attribute() {
            this->list.~array();
         }
         #pragma endregion

      public:
         std::array<value_type, 3> list = { 0 };
         struct {
            value_type health;
            value_type magicka;
            value_type stamina;
         };
         struct {
            value_type h;
            value_type m;
            value_type s;
         };
   };
}