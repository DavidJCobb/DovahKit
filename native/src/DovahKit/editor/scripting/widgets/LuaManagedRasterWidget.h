#pragma once
#include <QWidget>

#include "../systems/lua_managed_resources.h"

class LuaManagedRasterWidget : public QWidget {
   Q_OBJECT;
   using LMR  = editor_script::LuaManagedResource;
   using LMRH = editor_script::LuaManagedResourceHandle;
   public:
      LuaManagedRasterWidget(QWidget* parent = nullptr);

      inline QSize desiredSize() const noexcept { return this->_desiredSize; }
      void setDesiredSize(const QSize&) noexcept;
      void setDesiredWidth(int) noexcept;
      void setDesiredHeight(int) noexcept;

      inline LMRH resource() const noexcept { return this->_resource; }
      void setResource(const LMRH&);

   protected:
      LMRH _resource;
      QSize _desiredSize;

      virtual bool hasHeightForWidth() const override { return true; }
      virtual int heightForWidth(int w) const override;
      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;

      virtual void paintEvent(QPaintEvent*) override;

      const QPixmap _getPixmap() const noexcept;
};