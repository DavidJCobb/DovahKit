#pragma once
#include <QString>
#include "dovah/data/perk_entry_points.h"

namespace editor::localize {
   extern QString perk_entry_point_explanation(dovah::perk_entry_point);

   namespace elaborated {
      // For when the text is too long to fit in a compact UI; the above function will invite the 
      // user to use the What's This? button to view the string returned by this function.
      extern QString perk_entry_point_explanation(dovah::perk_entry_point);
   }
}