// define the `ENUMERATION_TYPE` macro as the fully-qualfiied typename of the enum.
// then include this header. it will define macro `STRING` taking a string-literal 
// parameter.

#pragma region macro boilerplate
   #define STR_(x) #x
   #define STR(x) STR_(x)
#pragma endregion
#define TRANSLATION_KEY STR(ENUMERATION_TYPE)

#define STRING(t) QCoreApplication::translate(TRANSLATION_KEY, t)