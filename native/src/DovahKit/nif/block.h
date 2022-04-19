#pragma once
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include "blocks/_DKVulkanInterface.h"
#include "types/NiTransform.h"

namespace nifDK {
   class file_reader;

   enum class block_type {
      unknown,
      //
      // block types not yet implemented:
      //
      BSDismemberSkinInstance,
      BSEffectShaderProperty,
      BSFadeNode,
      BSLightingShaderProperty,
      BSShaderFlags,
      BSShaderFlags2,
      BSShaderLightingProperty,
      BSShaderProperty,
      BSShaderTextureSet,
      BSShaderType,
      BSSkyShaderProperty,
      BSWaterShaderProperty,
      BSXFlags,
      NiAlphaProperty,
      NiAVObject, // extends NiObjectNET
      NiBillboardNode,
      NiGeometry,
      NiNode,
      NiObject,
      NiObjectNET, // extends NiObject
      NiSkinData,
      NiSkinInstance,
      NiSwitchNode,
      NiTriBasedGeom,
      NiTriShape, // extends NiTriBasedGeom
      NiTriShapeData,
      SkyrimShaderPropertyFlags1,
      SkyrimShaderPropertyFlags2,
      SkyrimWaterShaderFlags,
   };

   class block : public virtual block_interfaces::_DKVulkanInterface {
      public:
         block_type type = block_type::unknown;

         virtual void parse(file_reader&) = 0;
   };

   class extra_data_list {
      public:
         
   };

   namespace block_types {
      class unknown_block;
   }
}