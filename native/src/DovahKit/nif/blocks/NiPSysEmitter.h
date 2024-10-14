#pragma once
#include "NiPSysModifier.h"
#include "../types/NiColor.h"

namespace nifDK::block_types {
   class NiPSysEmitter : public NiPSysModifier {
      public:
         static constexpr const char* const type_name = "NiPSysEmitter";
      public:
         struct {
            float base     = 0;
            float variance = 0;
         } speed;
         struct {
            float base     = 0;
            float variance = 0;
         } declination;
         struct {
            float base     = 0;
            float variance = 0;
         } planar_angle;
         NiColorA initial_color;
         struct {
            float base     = 0;
            float variance = 0;
         } radius;
         struct {
            float base     = 0;
            float variance = 0;
         } lifespan;

         virtual void parse(file_reader&) override;
   };
}