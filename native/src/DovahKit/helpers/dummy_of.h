#pragma once
#include <concepts>
#include <type_traits>

namespace cobb {
   //
   // Creates an empty no-op object that mimics the constructor signatures and 
   // operator overloads of a given "underlying type." Instances of the dummy 
   // type always compare non-equal to other objects.
   //
   template<typename Underlying>
   class dummy_of {
      public:
         using underlying_type = Underlying;

      public:
         constexpr dummy_of(const underlying_type&) requires(std::is_copy_constructible_v<underlying_type>) {}
         constexpr dummy_of(underlying_type&&) requires(std::is_move_constructible_v<underlying_type>) {}
         constexpr dummy_of(const dummy_of&) requires(std::is_copy_constructible_v<underlying_type>) {}
         constexpr dummy_of(dummy_of&&) requires(std::is_move_constructible_v<underlying_type>) {}

         template<typename... Args> requires std::is_constructible_v<underlying_type, Args...>
         constexpr dummy_of(Args&&...) {}

         constexpr dummy_of& operator=(const dummy_of&) { return *this; }
         constexpr dummy_of& operator=(dummy_of&&) { return *this; }

         constexpr bool operator==(const dummy_of&) const noexcept { return false; }
         
         #pragma push_macro("COBB_DUMMY_OF_OPERAND")
         #pragma push_macro("COBB_DUMMY_OF_OPERAND_ASSIGN")
         #define COBB_DUMMY_OF_OPERAND(symbol) \
            template<typename Operand> requires requires(const underlying_type& x, const Operand& y) { { x symbol y }; } \
            constexpr auto operator##symbol(const Operand&) const { \
               using result = std::invoke_result_t<decltype(&underlying_type::operator##symbol), underlying_type, Operand>; \
               if constexpr (std::is_same_v<result, void>) { \
                  return; \
               } else { \
                  return result{}; \
               } \
            }; \
            template<> requires requires(const underlying_type& x, const underlying_type& y) { { x symbol y }; } \
            constexpr auto operator##symbol<dummy_of>(const dummy_of&) const { \
               using result = std::invoke_result_t<decltype(&underlying_type::operator##symbol), underlying_type, underlying_type>; \
               if constexpr (std::is_same_v<result, void>) { \
                  return; \
               } else { \
                  return result{}; \
               } \
            };
         #define COBB_DUMMY_OF_OPERAND_ASSIGN(symbol) \
            template<typename Operand> requires requires(underlying_type& x, const Operand& y) { { x symbol##= y } -> std::same_as<underlying_type&>; } \
            constexpr dummy_of operator##symbol##=(const Operand&) { \
               return *this; \
            }; \
            template<> requires requires(underlying_type& x, const underlying_type& y) { { x symbol##= y } -> std::same_as<underlying_type&>; } \
            constexpr dummy_of operator##symbol##=<dummy_of>(const dummy_of&) { \
               return *this; \
            };

         COBB_DUMMY_OF_OPERAND(+);
         COBB_DUMMY_OF_OPERAND(-);
         COBB_DUMMY_OF_OPERAND(*);
         COBB_DUMMY_OF_OPERAND(/);
         COBB_DUMMY_OF_OPERAND(%);
         COBB_DUMMY_OF_OPERAND(&);
         COBB_DUMMY_OF_OPERAND(|);
         COBB_DUMMY_OF_OPERAND(^);
         COBB_DUMMY_OF_OPERAND(<<);
         COBB_DUMMY_OF_OPERAND(>>);
         template<typename Operand> requires requires(underlying_type& x, const Operand& y) { { x = y } -> std::same_as<underlying_type&>; }
         constexpr dummy_of operator=(const Operand&){
            return *this;
         }
         template<> requires requires(underlying_type& x, const underlying_type& y) { { x = y } -> std::same_as<underlying_type&>; }
         constexpr dummy_of operator=<dummy_of>(const dummy_of&) {
            return *this;
         };
         COBB_DUMMY_OF_OPERAND_ASSIGN(+);
         COBB_DUMMY_OF_OPERAND_ASSIGN(-);
         COBB_DUMMY_OF_OPERAND_ASSIGN(*);
         COBB_DUMMY_OF_OPERAND_ASSIGN(/);
         COBB_DUMMY_OF_OPERAND_ASSIGN(%);
         COBB_DUMMY_OF_OPERAND_ASSIGN(&);
         COBB_DUMMY_OF_OPERAND_ASSIGN(|);
         COBB_DUMMY_OF_OPERAND_ASSIGN(^);
         COBB_DUMMY_OF_OPERAND_ASSIGN(<<);
         COBB_DUMMY_OF_OPERAND_ASSIGN(>>);

         #undef COBB_DUMMY_OF_OPERAND_ASSIGN
         #undef COBB_DUMMY_OF_OPERAND
         #pragma pop_macro("COBB_DUMMY_OF_OPERAND_ASSIGN")
         #pragma pop_macro("COBB_DUMMY_OF_OPERAND")

         //
         // Unary operators:
         //
         template<typename Operand> requires requires(const underlying_type& x) { { +x }; }
         constexpr auto operator+() const {
            return std::invoke_result_t<decltype(&underlying_type::operator+), underlying_type>{};
         };
         template<typename Operand> requires requires(const underlying_type& x) { { -x }; }
         constexpr auto operator-() const {
            return std::invoke_result_t<decltype(&underlying_type::operator-), underlying_type>{};
         };
         template<typename Operand> requires requires(const underlying_type& x) { { !x }; }
         constexpr auto operator!() const {
            return std::invoke_result_t<decltype(&underlying_type::operator!), underlying_type>{};
         };
   };

   template<bool Condition, typename Underlying>
   using dummy_type_if_false = std::conditional_t<Condition, Underlying, dummy_of<Underlying>>;
}