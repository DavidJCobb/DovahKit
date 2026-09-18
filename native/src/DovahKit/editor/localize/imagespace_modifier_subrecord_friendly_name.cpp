#include "./imagespace_modifier_subrecord_friendly_name.h"
#include <QCoreApplication>
#include "helpers/qt/strings.h"
#include "dovah/forms/ImagespaceModifier.h" // for indexed subrecords

namespace editor::localize {
   extern QString imagespace_modifier_subrecord_friendly_name(uint32_t signature) {
      switch (signature) {
         #define CASE(subrecord, text) case subrecord: return QCoreApplication::translate("IMAD subrecord to property name", text); break
         CASE('BNAM', "Basic Blur Radius");
         CASE('VNAM', "Double Vision Strength");
         CASE('TNAM', "Tint Color");
         CASE('NAM3', "Fade Color");
         CASE('RNAM', "Radial Blur Strength");
         CASE('SNAM', "Radial Blur Ramp-Up");
         CASE('UNAM', "Radial Blur Inner Radius");
         CASE('NAM1', "Radial Blur Ramp-Down");
         CASE('NAM2', "Radial Blur Outer Radius");
         CASE('WNAM', "Depth of Field Strength");
         CASE('XNAM', "Depth of Field Distance");
         CASE('YNAM', "Depth of Field Range");
         CASE('NAM4', "Motion Blur Strength");
         #undef CASE
         #define CASE(index, text) \
            case dovah::loaded_forms::ImagespaceModifier::interpolator_subrecord(index):        return QCoreApplication::translate("IMAD subrecord to property name", text " (Multiply)"); break; \
            case dovah::loaded_forms::ImagespaceModifier::interpolator_subrecord(index + 0x40): return QCoreApplication::translate("IMAD subrecord to property name", text " (Add)"); break;
         CASE( 0, "HDR: Eye Adapt Speed");
         CASE( 1, "HDR: Bloom Blur Radius");
         CASE( 2, "HDR: Bloom Threshold");
         CASE( 3, "HDR: Bloom Scale");
         CASE( 4, "HDR: Target Luminescence Min");
         CASE( 5, "HDR: Target Luminescence Max");
         CASE( 6, "HDR: Sunlight Scale");
         CASE( 7, "HDR: Sky Scale");
         CASE(17, "Cinematic: Saturation");
         CASE(18, "Cinematic: Brightness");
         CASE(19, "Cinematic: Contrast");
         CASE(20, "Cinematic: Unused");
         #undef CASE
         default:
            {
               auto index = dovah::loaded_forms::ImagespaceModifier::index_from_interpolator_subrecord(signature);
               bool add   = false;
               if (index >= 0x40) {
                  add = true;
                  index -= 0x40;
               }
               if (index >= 8 && index <= 16) {
                  index -= 8;
                  if (add)
                     return QCoreApplication::translate("IMAD subrecord to property name", "Unknown %1 (Multiply)").arg(index);
                  return QCoreApplication::translate("IMAD subrecord to property name", "Unknown %1 (Add)").arg(index);
               }
            }
            break;
      }
      return QObject::tr("subrecord %1").arg(cobb::qt::four_cc_to_string(signature));
   }
}