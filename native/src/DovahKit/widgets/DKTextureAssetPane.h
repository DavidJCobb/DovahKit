#pragma once
#include <QElapsedTimer>
#include <QFrame>
#if !defined(QT_DESIGNER_LIB)
   #include "../editor/asset_manager/asset.h"
#else
   class DovahKitAsset;
#endif

class DKTextureAssetPane : public QFrame {
   Q_OBJECT;
   private:
      #if defined(QT_DESIGNER_LIB)
      struct DovahKitAssetHandle {
         using construct_type = void*;
      };
      #endif

   public:
      DKTextureAssetPane(QWidget* parent = nullptr);

      bool hasAsset() const noexcept;
      void setAsset(const QString& path);
      void setAsset(DovahKitAssetHandle::construct_type);

   protected slots:
      void _onAssetReady();

   protected:
      enum class Render {
         Null,
         Loading,
         Asset,
      };

      DovahKitAssetHandle _handle;
      Render _render = Render::Null;
      struct {
         QElapsedTimer elapsed;
         int updateID = 0;
      } _animation;

      void _drawLoadingSpinner(QPainter&, QRect);
      void _drawNullSymbol(QPainter&, QRect);

      void _startAnimation();
      void _stopAnimation();

      virtual void paintEvent(QPaintEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;
};