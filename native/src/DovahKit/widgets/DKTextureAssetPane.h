#pragma once
#include <QElapsedTimer>
#include <QFrame>
#include <QTimer>
#if !defined(QT_DESIGNER_LIB)
   #include "../editor/asset_manager/asset.h"
#else
   class DovahKitAsset;
#endif

namespace dovah {
   class form_stub;
}

class DKTextureAssetPane : public QFrame {
   Q_OBJECT;
   private:
      #if defined(QT_DESIGNER_LIB)
      struct DovahKitAssetReceptor {};
      struct DovahKitAssetTransport {};
      #endif

   public:
      DKTextureAssetPane(QWidget* parent = nullptr);

      bool hasAsset() const noexcept;
      void setAsset(const QString& path);
      void setAsset(dovah::form_stub*);
      void setAsset(DovahKitAssetTransport&&);
      void setAsset(const DovahKitAssetReceptor&);

      // Throttle the loading of new assets (from a path or a form stub). Useful for if 
      // you've hooked this DKTextureAssetPane up to a QComboBox or something else that 
      // users can scroll through very quickly one item at a time.
      //
      // A throttle of 75ms is good for the case  of the user clicking into a QComboBox 
      // like that and just holding the arrow key to blitz through its contents, but it 
      // won't generally catch button mashing.
      inline bool isThrottleEnabled() const noexcept { return this->_throttle.enabled; }
      inline uint throttleTime() const noexcept { return this->_throttle.ms; }

   public slots:
      void setThrottleEnabled(bool);
      void setThrottleTime(uint ms);

   protected slots:
      void _onAssetUnloaded();
      void _onTargetAssetHandled();
      void _onRenderAssetHandled();

   protected:
      enum class Render {
         Null,
         Loading,
         Failed,
         Asset,
      };

      Render _render = Render::Null;
      struct {
         DovahKitAssetReceptor target;
         DovahKitAssetReceptor render;
      } _receptors;
      struct {
         QElapsedTimer elapsed;
         int updateID = 0;
      } _animation;
      struct {
         bool enabled = false;
         uint ms      = 100;
         QTimer  timer;
         QString path;
         dovah::form_stub* stub = nullptr;
      } _throttle;

      void _clearThrottleData();
      bool _hasThrottleData() const noexcept;

      void _drawLoadingSpinner(QPainter&, QRect);
      void _drawNullSymbol(QPainter&, QRect);
      void _drawFailSymbol(QPainter&, QRect);

      void _startAnimation();
      void _stopAnimation();

      virtual void paintEvent(QPaintEvent* event) override;
      virtual void timerEvent(QTimerEvent* event) override;
};