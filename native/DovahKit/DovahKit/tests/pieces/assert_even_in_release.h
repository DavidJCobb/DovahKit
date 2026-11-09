#if _DEBUG
   #include <cassert>
   #define assert_even_in_release(expression) assert(expression)
#else
   #include <exception>

   #ifndef GUARD_ASSERT_EVEN_IN_RELEASE_H
      #define GUARD_ASSERT_EVEN_IN_RELEASE_H

      // Used so that `assert_even_in_release` expressions don't get optimized to jump to the same address on 
      // failure, which complicates inspecting them in a debugger.
      #ifdef _MSC_VER
         [[msvc::noinline]]
      #endif
      extern void _____assert_even_in_release_dummy(unsigned int);
   #endif

   #define assert_even_in_release(expression)  ( (void) ((!!(expression)) || (_____assert_even_in_release_dummy(__LINE__), std::terminate(), 0)) )
#endif
