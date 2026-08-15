#pragma once
#include <concepts>
#include <string>
#include "helpers/class_array.h"
#pragma region block types
   #include "unknown.h"
   //
   // Includes are organized by class hierarchy:
   //
   #include "NiObject.h"
      #include "bhkRefObject.h"
         #include "bhkSerializable.h"
            #include "bhkConstraint.h"
            #include "bhkShape.h"
               #include "bhkBvTreeShape.h"
                  #include "bhkMoppBvTreeShape.h"
               #include "bhkConvexSweepShape.h"
               #include "bhkShapeCollection.h"
                  #include "bhkListShape.h"
                  #include "bhkNiTriStripsShape.h"
               #include "bhkSphereRepShape.h"
                  #include "bhkConvexShape.h"
                     #include "bhkBoxShape.h"
                     #include "bhkCapsuleShape.h"
                     #include "bhkConvexVerticesShape.h"
                     #include "bhkSphereShape.h"
                  #include "bhkMultiSphereShape.h"
               #include "bhkTransformShape.h"
                  #include "bhkConvexTransformShape.h"
            #include "bhkWorldObject.h"
               #include "bhkEntity.h"
                  #include "bhkRigidBody.h"
                     #include "bhkRigidBodyT.h"
               #include "bhkPhantom.h"
                  #include "bhkShapePhantom.h"
                     #include "bhkSimpleShapePhantom.h"
      #include "BSMultiBound.h"
      #include "BSShaderTextureSet.h"
      #include "NiCollisionObject.h"
      #include "NiExtraData.h"
         #include "BSBound.h"
         #include "BSFurnitureMarkerNode.h"
         #include "BSInvMarker.h"
         #include "NiIntegerExtraData.h"
            #include "BSXFlags.h"
         #include "NiStringExtraData.h"
      #include "NiGeometryData.h"
         #include "NiTriBasedGeomData.h"
            #include "NiTriShapeData.h"
            #include "NiTriStripsData.h"
      #include "NiInterpolator.h"
      #include "NiObjectNET.h"
         #include "NiAVObject.h"
            #include "BSTriShape.h"
            #include "NiDynamicEffect.h"
            #include "NiGeometry.h"
               #include "NiParticles.h"
                  #include "NiParticleSystem.h"
               #include "NiTriBasedGeom.h"
                  #include "BSLODTriShape.h"
                  #include "NiTriShape.h"
                  #include "NiTriStrips.h"
            #include "NiNode.h"
               #include "BSFadeNode.h"
               #include "BSLeafAnimNode.h"
               #include "BSMasterParticleSystem.h"
               #include "BSMultiBoundNode.h"
               #include "BSOrderedNode.h"
               #include "BSRangeNode.h"
                  #include "BSBlastNode.h"
                     #include "BSDamageStage.h"
               #include "BSValueNode.h"
               #include "NiBillboardNode.h"
               #include "NiSwitchNode.h"
         #include "NiProperty.h"
            #include "NiAlphaProperty.h"
            #include "NiShadeProperty.h"
               #include "BSShaderProperty.h"
                  #include "BSEffectShaderProperty.h"
                  #include "BSLightingShaderProperty.h"
      #include "NiParticlesData.h"
         #include "NiPSysData.h"
      #include "NiPSysEmitterCtlrData.h"
      #include "NiPSysModifier.h"
         #include "NiPSysEmitter.h"
            #include "NiPSysVolumeEmitter.h"
      #include "NiSkinData.h"
      #include "NiSkinInstance.h"
         #include "BSDismemberSkinInstance.h"
      #include "NiSkinPartition.h"
      #include "NiTimeController.h"
         #include "NiInterpController.h"
            #include "NiSingleInterpController.h"
               #include "NiFloatInterpController.h"
                  #include "BSLightingShaderPropertyFloatController.h"
               #include "NiPSysModifierCtlr.h"
                  #include "NiPSysEmitterCtlr.h"
                     #include "BSPSysMultiTargetEmitterCtlr.h"
#pragma endregion

namespace nifDK {
   using all_block_types = cobb::class_array<
      block_types::unknown_block,
      //
      block_types::bhkBoxShape,
      block_types::bhkBvTreeShape,
      block_types::bhkCapsuleShape,
      block_types::bhkConstraint,
      block_types::bhkConvexShape,
      block_types::bhkConvexSweepShape,
      block_types::bhkConvexTransformShape,
      block_types::bhkConvexVerticesShape,
      block_types::bhkEntity,
      block_types::bhkListShape,
      block_types::bhkMoppBvTreeShape,
      block_types::bhkMultiSphereShape,
      block_types::bhkNiTriStripsShape,
      block_types::bhkPhantom,
      block_types::bhkRefObject,
      block_types::bhkRigidBody,
      block_types::bhkRigidBodyT,
      block_types::bhkSerializable,
      block_types::bhkShape,
      block_types::bhkShapeCollection,
      block_types::bhkShapePhantom,
      block_types::bhkSimpleShapePhantom,
      block_types::bhkSphereRepShape,
      block_types::bhkSphereShape,
      block_types::bhkTransformShape,
      block_types::bhkWorldObject,
      block_types::BSBlastNode,
      block_types::BSBound,
      block_types::BSDamageStage,
      block_types::BSDismemberSkinInstance,
      block_types::BSEffectShaderProperty,
      block_types::BSFadeNode,
      block_types::BSFurnitureMarkerNode,
      block_types::BSInvMarker,
      block_types::BSLeafAnimNode,
      block_types::BSLightingShaderProperty,
      block_types::BSLightingShaderPropertyFloatController,
      block_types::BSLODTriShape,
      block_types::BSMasterParticleSystem,
      block_types::BSMultiBound,
      block_types::BSMultiBoundData,
      block_types::BSMultiBoundDataOBB,
      block_types::BSMultiBoundDataSphere,
      block_types::BSMultiBoundNode,
      block_types::BSOrderedNode,
      block_types::BSPSysMultiTargetEmitterCtlr,
      block_types::BSRangeNode,
      block_types::BSShaderProperty,
      block_types::BSShaderTextureSet,
      block_types::BSTriShape,
      block_types::BSValueNode,
      block_types::BSXFlags,
      block_types::NiAlphaProperty,
      block_types::NiAVObject,
      block_types::NiBillboardNode,
      block_types::NiCollisionObject,
      block_types::NiDynamicEffect,
      block_types::NiExtraData,
      block_types::NiFloatInterpController,
      block_types::NiGeometry,
      block_types::NiGeometryData,
      block_types::NiIntegerExtraData,
      block_types::NiInterpController,
      block_types::NiInterpolator,
      block_types::NiNode,
      block_types::NiObject,
      block_types::NiObjectNET,
      block_types::NiParticlesData,
      block_types::NiProperty,
      block_types::NiPSysData,
      block_types::NiPSysEmitter,
      block_types::NiPSysEmitterCtlr,
      block_types::NiPSysEmitterCtlrData,
      block_types::NiPSysModifier,
      block_types::NiPSysModifierCtlr,
      block_types::NiPSysVolumeEmitter,
      block_types::NiShadeProperty,
      block_types::NiSingleInterpController,
      block_types::NiSkinData,
      block_types::NiSkinInstance,
      block_types::NiSkinPartition,
      block_types::NiStringExtraData,
      block_types::NiSwitchNode,
      block_types::NiTimeController,
      block_types::NiTriBasedGeom,
      block_types::NiTriBasedGeomData,
      block_types::NiTriShape,
      block_types::NiTriShapeData,
      block_types::NiTriStrips,
      block_types::NiTriStripsData//,
   >;
}

#include "./_concepts.h"