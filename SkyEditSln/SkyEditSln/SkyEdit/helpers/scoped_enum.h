#pragma once

//
// For full details, see: /notes/scoped enums.txt
//

#define _SCOPE_ENUM_HELPER_01(name, c, ...) namespace _scoped_enums##c { __VA_ARGS__##; }; using _scoped_enums##c##::##name;

/// Given a name and an enum definition, this macro makes the enum scoped while retaining the ability to implicitly cast it: the macro defines the enum in a unique namespace, to avoid any possible conflicts stemming from enums' scope pollution; and then it uses a using declaration to pull the enum into the namespace containing the macro. The name given to the macro must match the name of the enum.
///
/// Note that if you use this macro, you CANNOT use single-line comments anywhere in the enum definition. Multi-line macro invocations get collapsed into single-line code. Use block comments instead. Semicolons anywhere in the enum definition may cause problems as well. Single-line comment tokens may cause problems even if they are inside of a block comment; MSVC's preprocessor sucks, and the command line switch to enable the experimental improved preprocessor doesn't seem to actually work in my copy of VS2019.
///
/// Note also that this often breaks IntelliSense and shows false errors. The code should still compile.
//
#define SCOPE_ENUM(name, ...) _SCOPE_ENUM_HELPER_01(name, __COUNTER__, __VA_ARGS__)

/*// Usage:
SCOPE_ENUM(Baz, enum Baz : int {
   a = 5,
   b = 3,
});
constexpr Baz test = Baz::a;
//*/