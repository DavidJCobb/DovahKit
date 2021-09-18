#pragma once
#include <array>
#include "concepts.h"

namespace cobb {
   namespace impl::enum_bitfield {
      extern constexpr auto dummy_array = std::array{ 0 }; // MSVC complains about enum_bitfield even if it's not instantiated... -_-
   }

   //
   // Use to give data structures a compile-time-specified bitmask of values from a 
   // discontiguous enum. Start by defining your enum type:
   // 
   //    enum class my_enum {
   //       foo,
   //       bar,
   //       baz = 50,
   //    };
   // 
   //    using my_mask = enum_bitfield<std::array{
   //       my_enum::foo,
   //       my_enum::bar,
   //       my_enum::baz,
   //    }>;
   // 
   // Then, you can define instances at compile-time:
   // 
   //    auto a = my_mask();                                // == 0
   //    auto b = my_mask::from<my_enum::foo>;              // == 1
   //    auto c = my_mask::from<my_enum::foo, my_enum:bar>; // == 3 == 1 | 2
   //    auto d = my_mask::from<my_enum::baz>;              // == 4
   //    auto e = my_mask::from<(my_enum)999>;              // compile error
   // 
   // To check, at run-time, if a mask contains a given item:
   // 
   //    bool foo = a.contains(my_enum::foo);
   //
   template<auto _values = impl::enum_bitfield::dummy_array, typename M = uint32_t > requires (cobb::is_std_array_instance<_values> && std::integral<M>)
   class enum_bitfield {
      public:
         using item_type = std::decay<decltype(_values)>::value_type;
         using mask_type = M;
      protected:
         static constexpr int available_bits = sizeof(mask_type) * 8;

         template<item_type g> static consteval int bit_index_for() {
            auto it = std::find(_values.begin(), _values.end(), g);
            if (it == _values.end())
               return -1;
            return std::distance(_values.begin(), it);
         }

      protected:
         mask_type mask = 0;

      public:
         constexpr enum_bitfield() {}
         constexpr enum_bitfield(const enum_bitfield& o) : mask(o.mask) {}
         constexpr enum_bitfield(mask_type t) : mask(t) {}

      protected:
         template<item_type g> static consteval mask_type from_helper() {
            constexpr auto m = bit_index_for<g>();
            static_assert(m >= 0,              "unrecognized element provided to this enum_bitfield");
            static_assert(m <  available_bits, "insufficient bits in the mask for this enum_bitfield");
            return 1 << m;
         }
         template<item_type g, item_type ...G> requires (sizeof...(G) > 0) static consteval mask_type from_helper() {
            return from_helper<g>() | from_helper<G...>();
         }
      public:
         // Initialize a simple_enum_bitfield at compile-time with a list of values.
         template<item_type ...G> static consteval enum_bitfield from() {
            return enum_bitfield(from_helper<G...>());
         }

         inline constexpr mask_type as_mask() const noexcept { return this->mask; }
         constexpr bool contains(item_type g) const noexcept {
            for (size_t i = 0; i < _values.size(); ++i)
               if (_values[i] == g)
                  return this->mask & (1 << i);
            return false;
         }
   };

   // Create a flags-mask out of an enum. Features the ability to initialize a mask's value at 
   // compile-time and check that the specified flags are valid (enum values are non-negative 
   // and can fit in the chosen mask type).
   // 
   // Only suitable for enums with contiguous, non-negative elements starting from zero. For 
   // enums with discontiguous elements, use enum_bitfield above.
   template<typename T, typename MT = uint32_t> class simple_enum_bitfield {
      public:
         using item_type    = T;
         using mask_type    = MT;
         using integer_type = std::underlying_type_t<item_type>;
         static constexpr int available_bitcount = sizeof(mask_type) * 8;

         static constexpr mask_type item_to_mask(item_type e) noexcept {
            return mask_type(1) << (integer_type)e;
         }

         template<item_type e> struct item_is_in_bounds { // can't nest concepts into classes/structs, so this is the next best thing
            constexpr operator bool() const {
               return (integer_type)e < sizeof(mask_type) * 8;
            };
         };
         template<item_type e> struct item_is_valid { // can't nest concepts into classes/structs, so this is the next best thing
            constexpr operator bool() const {
               return (integer_type)e >= 0;
            };
         };

         template<item_type e> requires((bool)item_is_valid<e>{} && (bool)item_is_in_bounds<e>{})
         static constexpr mask_type item_as_mask = mask_type(1) << (integer_type)e;

      protected:
         mask_type mask = 0;

      protected:
         template<typename... ts> requires cobb::all_same_as<mask_type, ts...> // This is the only way to accept a variable number of mask_type arguments
         static constexpr mask_type or_all(ts... M) {
            return (M | ...);
         }

      public:
         constexpr simple_enum_bitfield() {}
         constexpr simple_enum_bitfield(const simple_enum_bitfield<T>& o) : mask(o.as_mask()) {}
         constexpr simple_enum_bitfield(mask_type m) : mask(m) {}

         inline constexpr mask_type as_mask() const noexcept { return this->mask; }
         inline constexpr bool contains(item_type e) const noexcept {
            return (this->mask & item_to_mask(e)) != 0;
         }

         static consteval simple_enum_bitfield<item_type> from_all() {
            return simple_enum_bitfield<item_type>(mask_type(-1));
         }

         // Initialize a simple_enum_bitfield at compile-time with a list of values. Validity of the 
         // values (i.e. non-negative; fits in the mask) will be checked at compile-time, and will be 
         // visible on any calls in IntelliSense, though the error message will not be intuitive (it 
         // will refer to a failure to produce a "constant expression").
         template<item_type... list> static consteval simple_enum_bitfield<item_type> from() {
            // IGNORE any IntelliSense errors on pack expansions in this function
            constexpr bool all_in_bounds = (item_is_in_bounds<list>{} && ...);
            constexpr bool all_are_valid = (item_is_valid<list>{} && ...);
            static_assert(all_in_bounds, "the simple_enum_bitfield used here doesn't have a large enough mask to store one of the specified items (note: error site should be indicated by a C3615, not by this error)");
            static_assert(all_are_valid, "one or more of the items passed to simple_enum_bitfield::from was invalid (note: error site should be indicated by a C3615, not by this error)");
            if (!(all_in_bounds && all_are_valid)) throw; // static assertion failures don't stop consteval evaluation? wtf??
            return simple_enum_bitfield<item_type>(or_all(item_to_mask(list)...)); // using item_to_mask instead of item_as_mask avoids an extra compiler error if the assertions above fail
         }
   };
}