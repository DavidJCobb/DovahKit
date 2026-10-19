#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
   #define LU_RESTRICT __restrict
#else
   #define LU_RESTRICT
#endif