#pragma once
#include <optional>
#include <string>
#include <vector>
#include "types/NiTransform.h"

namespace nifDK {
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

   class block {
      public:
         block_type type = block_type::unknown;
   };

   class unknown_block : block {
      public:
         std::string type_name;
   };

   class extra_data_list {
      public:
         
   };

   struct NiTransform;

   namespace block_types {
      class NiAlphaProperty;
      class NiCollisionObject;
      class NiDynamicEffect;
      class NiTimeController;
      // ...

      class NiAVObject;

      class NiObject {};
      class NiObjectNET : public NiObject {
         public:
            // <add name="Skyrim Shader Type" type="BSLightingShaderPropertyShaderType" vercond="User Version >= 12" cond="BSLightingShaderProperty">Configures the main shader path</add>
            std::string       name; // uint32_t length; chars;
            extra_data_list   extra;
            NiTimeController* controller = nullptr;
      };
      class NiAVObject : public NiObjectNET {
         public:
            uint32_t flags = 0;
            // <add name="Unknown Short 1" type="ushort" default="8" ver1="20.2.0.7" vercond="(User Version >= 11) &amp;&amp; (User Version 2 > 26)" >Unknown Flag</add>
            NiTransform transform;
            NiCollisionObject* collision = nullptr;
      };
      class NiNode : public NiAVObject {
         public:
            std::vector<NiAVObject*>      children;
            std::vector<NiDynamicEffect*> effects;
      };
      class BSFadeNode : public NiNode {};

      class NiGeometryData;
      class NiSkinInstance;

      class NiGeometry : public NiAVObject {
         public:
            struct material {
               std::string name;
               void* extra = nullptr;
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
               void* shader = nullptr;
               void* alpha  = nullptr;
            } properties;

      };
      class NiTriBasedGeom : public NiGeometry {};
      class NiTriShape : public NiTriBasedGeom {};
   }
}