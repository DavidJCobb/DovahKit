
//
// Structs are not equality-comparable by default; giving a struct a default equality 
// comparison doesn't opt any nested anonymous structs into equality comparisons; and 
// there's no way to define or default an equality comparison for a type without the 
// ability to write out its name.
// 
// You could work around that by
// 
//    struct __dummy_name {
//       constexpr bool operator==(const __dummy_name&) const = default;
// 
//       int field01;
//    } name;
// 
// but that gets ugly when you have multiple anonymous structs:
// 
//    struct __magnitude {
//       constexpr bool operator==(const __magnitude&) const = default;
//       float yaw;
//       float pitch;
//    } magnitude;
//    struct __range {
//       constexpr bool operator==(const __range&) const = default;
//       axis_mapping x;
//       axis_mapping y;
//    } range;
// 
// There's tons of repetition, there's class names (that you shouldn't have even had 
// to create!) that get duplicated all over the place, and it just looks messy. So 
// let's use macros instead:
// 
//    __anonymous_struct {
//       __anonymous_default_equality;
//       float yaw;
//       float pitch;
//    } magnitude;
//    __anonymous_struct {
//       __anonymous_default_equality;
//       axis_mapping x;
//       axis_mapping y;
//    } range;
// 
// We here (ab)use templating and macros, which means that the following rules are 
// in effect:
// 
//  - Do not use the __COUNTER__ macro between opening a macro-enhanced anonymous 
//    struct and invoking the macro to default its equality comparison.
// 
//  - Do not define, in the outer struct, a struct named __cobb_anon_counter.
// 
//  - Do not define, in the outer struct, a struct whose name follows the pattern 
//    __cobb_anon_%d e.g. __cobb_anon_1, etc..
//
// This macro header has a corresponding "filename.undef.h" file, which you should 
// include at the very end of any file using these macros, so as to keep them from 
// cross-contaminating too much else.
//

#pragma push_macro("MACRO_CONCAT_IMPL")
#pragma push_macro("MACRO_CONCAT")
#pragma push_macro("__anonymous_struct_impl")
#pragma push_macro("__anonymous_struct")
#pragma push_macro("__anonymous_default_equality_impl")
#pragma push_macro("__anonymous_default_equality")

#include "./macro_concat.h"

#define __anonymous_struct_impl(n) \
   MACRO_CONCAT(__cobb_anon_, n);                   \
   template<int I> struct __cobb_anon_counter;      \
   template<> struct __cobb_anon_counter< n > {     \
      using type = MACRO_CONCAT(__cobb_anon_, n);   \
   };                                               \
   template<> struct __cobb_anon_counter< n + 1 > { \
      using type = MACRO_CONCAT(__cobb_anon_, n);   \
   };                                               \
   struct MACRO_CONCAT(__cobb_anon_, n)
#define __anonymous_struct __anonymous_struct_impl(__COUNTER__)

#define __anonymous_default_equality_impl(n) constexpr bool operator==(const __cobb_anon_counter< n >::type& other) const = default;
#define __anonymous_default_equality __anonymous_default_equality_impl(__COUNTER__)

/*
  
  Given a macro invocation like this:
  
     struct outer {
        struct anonymous_struct {
           anonymous_default_equality;
           int data = 5;
        } foo;
        struct anonymous_struct {
           anonymous_default_equality;
           int data = 5;
        } bar;
     };
  
  The code generated looks like this:

     struct outer {
        struct __cobb_anon_1; // forward-declare name for our "anonymous" struct using __COUNTER__
        //
        template<int I> struct __cobb_anon_counter; // forward-declare template that we'll use to reference its name
        //
        template<> struct __cobb_anon_counter<1 + 1> { // template-specialize a way to access our name given the next __COUNTER__ value
           using type = __cobb_anon_1;
        };
        //
        struct __cobb_anon_1 { // begin definition of our "anonymous" struct
           constexpr bool operator==(const __cobb_anon_counter<2>::type& other) const = default; // next __COUNTER__ value

           int data = 5;
        } foo;

        struct __cobb_anon_3;
        //
        template<int I> struct __cobb_anon_counter; // multiple forward-declarations are fine, i believe
        //
        template<> struct __cobb_anon_counter<3 + 1> {
           using type = __cobb_anon_3;
        };
        //
        struct __cobb_anon_3 {
           constexpr bool operator==(const __cobb_anon_counter<4>::type& other) const = default;

           int data = 5;
        } bar;
     }

*/