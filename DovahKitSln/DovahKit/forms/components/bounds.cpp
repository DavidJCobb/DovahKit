#include "bounds.h"
#include "../../esp/TESPlugin.h"

void ObjectBounds::load(TESPluginSubrecord& subrecord) {
   if (!subrecord.is_in_bounds(12))
      return;
   subrecord.unchecked_read(this->min.x);
   subrecord.unchecked_read(this->min.y);
   subrecord.unchecked_read(this->min.z);
   subrecord.unchecked_read(this->max.x);
   subrecord.unchecked_read(this->max.y);
   subrecord.unchecked_read(this->max.z);
}
/*static*/ void ObjectBounds::generateUseInfo(TESPluginSubrecord&, FormStub*) {
   return; // no use info to generate
}