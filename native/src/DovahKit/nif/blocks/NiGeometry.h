#pragma once
#include "NiAVObject.h"

namespace nifDK::block_types {
   class BSShaderProperty;
   class NiAlphaProperty;
   class NiGeometryData;
   class NiSkinInstance;

   class NiGeometry : public NiAVObject {
      public:
         static constexpr const char* const type_name = "NiGeometry";
      public:
         struct material {
            std::string name;
            int32_t extra = -1;
         };
      public:
         NiGeometryData* data = nullptr;
         NiSkinInstance* skin = nullptr;
         struct { // from versions 10.0.1.0 to 20.1.0.3
            std::string name;
            int32_t     extra = -1;
         } shader;
         struct {
            std::vector<material> list;
            int32_t active = -1;
            bool    needs_update;
         } materials;
         struct {
            BSShaderProperty* shader = nullptr;
            NiAlphaProperty*  alpha  = nullptr;
         } properties;

         virtual void parse(file_reader&) override;
   };
}