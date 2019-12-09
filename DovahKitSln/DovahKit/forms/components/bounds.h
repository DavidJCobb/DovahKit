#pragma once
#include <string>
#include "../../helpers/vector3.h"

class FormStub;
class TESPluginSubrecord;

struct ObjectBounds {
   using point_type = cobb::vector3<int16_t>;

   point_type min;
   point_type max;
   //
   void  get_size(point_type& out) const noexcept {
      out = this->max - this->min;
   }
   float get_volume() const noexcept {
      point_type sizes;
      this->get_size(sizes);
      return (float)sizes.x * (float)sizes.y * (float)sizes.z;
   }
   //
   void load(TESPluginSubrecord&);
   static void generateUseInfo(TESPluginSubrecord&, FormStub*);
};