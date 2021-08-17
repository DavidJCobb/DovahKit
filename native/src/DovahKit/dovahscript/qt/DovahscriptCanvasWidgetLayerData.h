#pragma once
#include "../../ui/generic/CanvasWidget.h"

//
// Abstract base class for all CanvasWidgetLayerData objects that can be referred to 
// directly via a Lua userdata. This is separate and distinct from the similar class 
// CanvasWidgetLayerDataLuaManagedResource, as that class cannot actually be referred 
// to by a userdata; we elide it, with attempts to access it from script instead 
// retrieving the wrapped LuaManagedResource.
//
class DovahscriptCanvasWidgetLayerData : public CanvasWidgetLayerData {
   Q_OBJECT;
   public:
      std::atomic<bool> is_lua_referenced = false;
};