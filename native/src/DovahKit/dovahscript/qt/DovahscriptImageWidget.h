#pragma once
#include <QWidget>
#include "../core/subsystems/resources/DovahscriptResource.h"

class DovahscriptImageWidget : public QWidget {
   Q_OBJECT;
   using DSR  = dovahscript::DovahscriptResource;
   using DSRH = dovahscript::DovahscriptResourceHandle;
   public:
      DovahscriptImageWidget(QWidget* parent = nullptr);

      inline QSize desiredSize() const noexcept { return this->_desiredSize; }
      void setDesiredSize(const QSize&) noexcept;
      void setDesiredWidth(int) noexcept;
      void setDesiredHeight(int) noexcept;

      inline DSRH resource() const noexcept { return this->_resource; }
      void setResource(const DSRH&);

   protected:
      DSRH  _resource;
      QSize _desiredSize;

      virtual bool hasHeightForWidth() const override;
      virtual int heightForWidth(int w) const override;
      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;

      virtual void paintEvent(QPaintEvent*) override;

      const QPixmap _getPixmap() const noexcept;
};