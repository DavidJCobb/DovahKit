#if _DEBUG
   #include <cassert>
   #define assert_even_in_release(expression) assert(expression)
#else
   #include <exception>
   #define assert_even_in_release(expression)  ( (void) ((!!(expression)) || (std::terminate(), 0)) )
#endif
