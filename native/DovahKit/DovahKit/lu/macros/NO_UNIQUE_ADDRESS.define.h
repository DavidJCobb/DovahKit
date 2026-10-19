#if __has_cpp_attribute(msvc::no_unique_address) || __INTELLISENSE__
   #define LU_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]] [[no_unique_address]]
#else
   #define LU_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif