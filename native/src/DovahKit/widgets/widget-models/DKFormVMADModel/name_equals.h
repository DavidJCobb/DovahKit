#pragma once
#include <QString>

namespace DKFormVMADModelObjects {
   inline bool name_equals(QString a, QString b) {
      //
      // This is not a straightforward QString::toLower check because Bethesda's string table is 
      // only case-insensitive within ASCII.
      //
      size_t size = a.size();
      if (size != b.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         auto ac = a[(uint)i].unicode(); // it's very annoying that QString was implemented in such a manner as to make this cast necessary.
         auto bc = b[(uint)i].unicode();
         if (ac == bc)
            continue;
         if (ac >= 'a' && ac <= 'z')
            ac -= 0x20;
         if (bc >= 'a' && bc <= 'z')
            bc -= 0x20;
         if (ac != bc)
            return false;
      }
      return true;
   }
}