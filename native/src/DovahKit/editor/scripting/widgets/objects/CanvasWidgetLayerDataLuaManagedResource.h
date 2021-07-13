#pragma once
#include "../../../../ui/generic/CanvasWidget.h"
#include "../../systems/lua_managed_resources.h"

class CanvasWidgetLayerDataLuaManagedResource : public CanvasWidgetLayerData {
   Q_OBJECT;
   using LMRH = editor_script::LuaManagedResourceHandle;
   protected:
      LMRH _handle;

      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept;

   public:
      virtual QRect rect() const noexcept;

      LMRH resource();
      void setResource(LMRH);
};