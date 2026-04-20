#ifdef _MSC_VER
   #define COBB__FORCEINLINE [[msvc::forceinline]]
#elif __GNUG__
   #define COBB__FORCEINLINE [[gnu::always_inline]]
#else
   #define COBB__FORCEINLINE
#endif