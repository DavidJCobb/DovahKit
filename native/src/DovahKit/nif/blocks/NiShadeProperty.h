#pragma once
#include "NiProperty.h"

namespace nifDK::block_types {
   class NiShadeProperty : public NiProperty {
      public:
         static constexpr const char* const type_name = "NiShadeProperty";
      public:
         bool phong_shading = false; // old NIF versions only

         virtual void parse(file_reader&) override;
   };
}