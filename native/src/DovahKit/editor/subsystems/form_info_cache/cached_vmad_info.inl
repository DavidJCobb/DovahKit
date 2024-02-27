#pragma once
#include "./cached_vmad_info.h"

namespace dovahkit::subsystems::form_info_cache {
   constexpr bool cached_vmad_info::operator==(const cached_vmad_info& o) const {
      if (this == &o)
         return true;

      if (this->attached.size() != o.attached.size())
         return false;
      if (this->deleted.size() != o.deleted.size())
         return false;

      for (auto& a : this->attached) {
         bool found = false;
         for (auto& b : o.attached) {
            if (a == b) {
               found = true;
               break;
            }
         }
         if (!found)
            return false;
      }
      for (auto& a : this->deleted) {
         bool found = false;
         for (auto& b : o.deleted) {
            if (a == b) {
               found = true;
               break;
            }
         }
         if (!found)
            return false;
      }

      return true;
   }
}