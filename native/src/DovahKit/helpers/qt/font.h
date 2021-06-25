#pragma once
#include <QFont>

namespace cobb::qt {

   //
   // A QFont is a set of font properties, as well as an internal flags-mask indicating which 
   // properties have actually been set. There are cases where you may wish to clear properties 
   // from a QFont or query which properties are actually set, but there are no documented 
   // functions for doing so. That doesn't mean that it's impossible; it just means that it may 
   // become impossible in the future...
   //

   extern void clear_font_properties(QFont&, uint mask) noexcept;
   extern bool test_font_properties(const QFont&, uint mask) noexcept; // use QFont::ResolveProperty for mask bits
}