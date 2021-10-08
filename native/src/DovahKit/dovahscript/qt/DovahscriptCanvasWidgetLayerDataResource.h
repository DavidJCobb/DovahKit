#pragma once
#include "DovahscriptCanvasWidgetLayerData.h"
#include "../core/subsystems/resources.h"

//
// This class is meant to be transparent to Lua scripts. There is no API for it; rather, 
// when you directly assign a raster (or other resource) to a `layer.data` field, we will 
// seamlessly create an instance of this class to wrap the resource. Similarly, if a layer 
// is using an instance of this class as its data, reading the `layer.data` field will 
// return the underlying resource direectly.
//
class DovahscriptCanvasWidgetLayerDataResource final : public DovahscriptCanvasWidgetLayerData {
   Q_OBJECT;
   using DSRH = dovahscript::DovahscriptResourceUIHandle;
   protected:
      DSRH _handle;

      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept;

   public:
      virtual QRect rect() const noexcept;

      DSRH resource();
      void setResource(DSRH);
};