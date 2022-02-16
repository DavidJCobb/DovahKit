#pragma once
#include <concepts>
#include <string>
#include "helpers/class_list.h"
#pragma region block types
   #include "unknown.h"
   //
   #include "NiObject.h"
      #include "BSShaderTextureSet.h"
      #include "NiCollisionObject.h"
      #include "NiExtraData.h"
         #include "BSInvMarker.h"
         #include "NiIntegerExtraData.h"
            #include "BSXFlags.h"
         #include "NiStringExtraData.h"
      #include "NiGeometryData.h"
         #include "NiTriBasedGeomData.h"
            #include "NiTriShapeData.h"
      #include "NiObjectNET.h"
         #include "NiAVObject.h"
            #include "BSTriShape.h"
            #include "NiDynamicEffect.h"
            #include "NiGeometry.h"
               #include "NiTriBasedGeom.h"
                  #include "NiTriShape.h"
            #include "NiNode.h"
               #include "BSFadeNode.h"
         #include "NiProperty.h"
            #include "NiAlphaProperty.h"
            #include "NiShadeProperty.h"
               #include "BSShaderProperty.h"
                  #include "BSEffectShaderProperty.h"
                  #include "BSLightingShaderProperty.h"
      #include "NiSkinData.h"
      #include "NiSkinInstance.h"
         #include "BSDismemberSkinInstance.h"
      #include "NiSkinPartition.h"
      #include "NiTimeController.h"
#pragma endregion

namespace nifDK {
   using all_block_types = cobb::class_list<
      block_types::unknown_block,
      //
      block_types::BSDismemberSkinInstance,
      block_types::BSEffectShaderProperty,
      block_types::BSFadeNode,
      block_types::BSInvMarker,
      block_types::BSLightingShaderProperty,
      block_types::BSShaderProperty,
      block_types::BSShaderTextureSet,
      block_types::BSTriShape,
      block_types::BSXFlags,
      block_types::NiAlphaProperty,
      block_types::NiAVObject,
      block_types::NiCollisionObject,
      block_types::NiDynamicEffect,
      block_types::NiExtraData,
      block_types::NiGeometry,
      block_types::NiGeometryData,
      block_types::NiIntegerExtraData,
      block_types::NiNode,
      block_types::NiObject,
      block_types::NiObjectNET,
      block_types::NiProperty,
      block_types::NiShadeProperty,
      block_types::NiSkinData,
      block_types::NiSkinInstance,
      block_types::NiSkinPartition,
      block_types::NiStringExtraData,
      block_types::NiTimeController,
      block_types::NiTriBasedGeom,
      block_types::NiTriBasedGeomData,
      block_types::NiTriShape,
      block_types::NiTriShapeData//,
   >;

   template<typename T> concept block_type_has_name = requires {
      { T::type_name } -> std::same_as<const char* const&>;
   };
}
