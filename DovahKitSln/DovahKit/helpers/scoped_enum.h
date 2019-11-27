#pragma once

//
// For full details, see: /notes/scoped enums.txt
//

/// Given a name and an enum definition, this macro makes the enum scoped while retaining the ability to implicitly cast it: the macro defines the enum in a unique namespace, to avoid any possible conflicts stemming from enums' scope pollution; and then it uses a using declaration to pull the enum into the namespace containing the macro. The name given to the macro must match the name of the enum.
///
/// Note that you CANNOT use any comments in the enum definition. Single-line comments cause a syntax error due to how macros work; comments of any other kind confuse IntelliSense. You can use the SCOPED_ENUM_COMMENT("Text") macro as a jury-rigged one-line comment instead. You also must not have a trailing comma after the last enum value, lest you confuse MSVC's preprocessor.
///
/// See </notes/scoped enums.txt> for more information.
//
#define SCOPE_ENUM(name, ...) namespace _scoped_enums { namespace _##name { __VA_ARGS__##; }; }; using _scoped_enums::_##name##::##name;

/// This macro exists to avoid unknown issues in IntelliSense that cause block comments inside of SCOPE_ENUM to break the IntelliSense parser. The argument should be a string literal.
//
#define SCOPED_ENUM_COMMENT(text)

/*// Usage:
SCOPE_ENUM(Baz, enum Baz : int {
   a = 5,
   b = 3,
});
constexpr Baz test = Baz::a;
//*/