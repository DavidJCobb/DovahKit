#pragma once
#include "../../ui/generic/CanvasWidget.h"
#include "../core/subsystems/resources.h"

class DovahscriptCanvasWidgetLayerDataResource : public CanvasWidgetLayerData {
   Q_OBJECT;
   using DSRH = dovahscript::DovahscriptResourceHandle;
   protected:
      DSRH _handle;

      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept;

   public:
      virtual QRect rect() const noexcept;

      DSRH resource();
      void setResource(DSRH);
};