
#include "./macro_concat.h"

namespace cobb::impl {
   namespace _static_warning {
      struct true_type {};
      struct false_type {};
   }
}

#pragma push_macro("static_warning")

//
// Defines a macro that can be used in function bodies or struct bodies to 
// emit a warning at build-time if a condition is false; this is similar to 
// static_assert. In MSVC these warnings are only emitted with the compiler 
// switches /w3 or /w4, or /w14996 to change C4996 to a level-1 warning.
//                                                                                                   x
// DUE TO THE LIMITATIONS OF C MACROS, YOU MUST WRAP THE CONDIITION INSIDE 
// OF PARENTHESES, TO PREVENT ANY COMMAS FROM CONFUSING THE PREPROCESSOR.
//                                                                                                   x
// This macro could be fixed to not require those if one were to find some 
// version of <https://stackoverflow.com/a/2124385> that isn't completely 
// broken in Microsoft's preprocessors. I don't know if it's only broken in 
// IntelliSense (though that alone would disqualify its use here) or if it's 
// broken in MSVC proper, but it's broken even when I use /Zc:preprocessor.
// 
// The general goal would be to create something like
// 
//     #define ALL_BUT_LAST_VARARG(...) MACRO_FIRST_N_OF(MACRO_MINUS_1(MACRO_COUNT_VARARGS(...)))
// 
// Then, you could use ALL_BUT_LAST_VARARG to pluck out everything except 
// the warning message, and MACRO_LAST_1_OF(...) to get the warning message.
//
#define static_warning(cond, msg) \
   constexpr static struct [[maybe_unused]] MACRO_CONCAT(__static_warning_impl_, __LINE__) { \
      [[msvc::forceinline]] [[deprecated(msg)]] constexpr void _(const ::cobb::impl::_static_warning::false_type&) {} \
      [[msvc::forceinline]] constexpr void _(const ::cobb::impl::_static_warning::true_type&) {} \
      \
      [[msvc::forceinline]] constexpr MACRO_CONCAT(__static_warning_impl_, __LINE__)() { \
         if constexpr ( (cond) ) { \
            _(::cobb::impl::_static_warning::true_type{}); \
         } else { \
            _(::cobb::impl::_static_warning::false_type{}); \
         } \
      } \
   } MACRO_CONCAT(__static_warning_dummy_member, __LINE__) {};