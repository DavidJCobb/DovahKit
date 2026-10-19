#if __has_cpp_attribute(msvc::forceinline) || __INTELLISENSE__
   #define LU_FORCE_INLINE [[msvc::forceinline]]
#elif __has_cpp_attribute(gnu::always_inline)
   #define LU_FORCE_INLINE [[gnu::always_inline]]
#elif __has_cpp_attribute(clang::always_inline)
   #define LU_FORCE_INLINE [[clang::always_inline]]
#else
   #define LU_FORCE_INLINE 
#endif