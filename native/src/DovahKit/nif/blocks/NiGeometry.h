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
         struct {
            std::vector<material> list;
            std::optional<std::string> shader; // from versions 10.0.1.0 to 20.1.0.3
            int32_t active = -1;
         } materials;
         struct {
            BSShaderProperty* shader = nullptr;
            NiAlphaProperty*  alpha  = nullptr;
         } properties;

         virtual void parse(file_reader&) override;
   };
}