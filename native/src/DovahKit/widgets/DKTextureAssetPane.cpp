#include "DKTextureAssetPane.h"
#include <QPainter>
#include <QTimerEvent>
#if !defined(QT_DESIGNER_LIB)
   #include "../editor/asset_manager/asset_manager.h"
#endif

namespace {
   constexpr qint64 loading_anim_duration  = 2000;
   constexpr int    loading_anim_rings     = 3;
   constexpr int    loading_anim_thickness = 5;
   constexpr int    loading_render_size    = 150;

   constexpr int null_icon_render_bounds = loading_render_size;
   constexpr int null_icon_render_size   = null_icon_render_bounds * 0.66;
   constexpr int null_icon_line_width    = 10;

   constexpr int minimum_length = 75;
}

DKTextureAssetPane::DKTextureAssetPane(QWidget* parent) : QFrame(parent) {
   this->setFrameShadow(QFrame::Shadow::Sunken);
   this->setFrameShape(QFrame::Shape::WinPanel);
   this->setMinimumSize(minimum_length, minimum_length);
   //
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(&this->_handle, &DovahKitAssetReceptor::ready, this, &DKTextureAssetPane::_onAssetReady);
   #endif
}

bool DKTextureAssetPane::hasAsset() const noexcept {
   #if defined(QT_DESIGNER_LIB)
      return false;
   #else
      return this->_handle != nullptr;
   #endif
}
void DKTextureAssetPane::setAsset(const QString& path) {
   if (path.isEmpty()) {
      this->_render = Render::Null;
      #if !defined(QT_DESIGNER_LIB)
         this->_handle = nullptr;
      #endif
      this->update();
      return;
   }
   this->_render = Render::Loading;
   #if !defined(QT_DESIGNER_LIB)
      auto& am = DovahKitAssetManager::get();
      this->_handle = am.requestTexture(path);
      this->update();
   #endif
}
void DKTextureAssetPane::setAsset(DovahKitAssetTransport&& asset) {
   #if !defined(QT_DESIGNER_LIB)
      this->_handle = std::move(asset);
      if (this->_handle == nullptr) {
         this->_render = Render::Null;
      } else {
         this->_render = Render::Loading;
      }
      this->update();
   #endif
}
void DKTextureAssetPane::setAsset(const DovahKitAssetReceptor& other) {
   #if !defined(QT_DESIGNER_LIB)
      this->_handle = (DovahKitAsset*)other; // don't do direct assign, as that would bulldoze our receptor's flags
      if (this->_handle == nullptr) {
         this->_render = Render::Null;
      } else {
         this->_render = Render::Loading;
      }
      this->update();
   #endif
}

void DKTextureAssetPane::_onAssetReady() {
   this->_render = Render::Asset;
   this->_stopAnimation();
   this->update();
}

void DKTextureAssetPane::_drawLoadingSpinner(QPainter& p, QRect rect) {
   constexpr int margin = loading_anim_thickness + 2;
   //
   QPointF center = rect.center();
   auto    length = std::min(rect.width(), rect.height()) - margin;
   //
   this->_startAnimation();
   qint64 ms = this->_animation.elapsed.elapsed();
   //
   bool  looping  = ms >= loading_anim_duration;
   qreal progress = (qreal)(ms % loading_anim_duration) / loading_anim_duration;
   //
   QColor color = this->palette().color(QPalette::ColorRole::Highlight);
   QPen   pen;
   pen.setWidth(loading_anim_thickness);
   pen.setColor(color);
   //
   p.save();
   p.setBrush(Qt::NoBrush);
   p.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
   p.translate(center);
   if (length < loading_render_size) {
      auto scale = qreal(length) / loading_render_size;
      p.scale(scale, scale);
   }
   //
   for (int i = 0; i < loading_anim_rings; ++i) {
      if (i) {
         progress -= (qreal(1) / loading_anim_rings);
         if (progress < 0) {
            if (!looping)
               break;
            progress += qreal(1);
         }
      }
      auto radius = loading_render_size * progress / 2;
      //
      color.setAlphaF(qreal(1) - progress);
      pen.setColor(color);
      p.setPen(pen);
      p.drawEllipse({ 0, 0 }, radius, radius);
   }
   //
   p.restore();
}
void DKTextureAssetPane::_drawNullSymbol(QPainter& p, QRect rect) {
   constexpr int  margin    = null_icon_line_width  + 2;
   constexpr auto halfwidth = null_icon_render_size / 2;
   //
   QPointF center = rect.center();
   auto    length = std::min(rect.width(), rect.height()) - margin;
   //
   QColor color = this->palette().color(QPalette::ColorRole::Dark);
   QPen   pen;
   pen.setWidth(null_icon_line_width);
   pen.setColor(color);
   pen.setCapStyle(Qt::PenCapStyle::FlatCap);
   //
   p.save();
   p.setBrush(Qt::NoBrush);
   p.setPen(pen);
   p.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
   p.translate(center);
   if (length < null_icon_render_bounds) {
      auto scale = qreal(length) / null_icon_render_bounds;
      p.scale(scale, scale);
   }
   p.drawEllipse({ 0, 0 }, null_icon_render_size / 2, null_icon_render_size / 2);
   p.drawLine(halfwidth, -halfwidth, -halfwidth, halfwidth);
}

void DKTextureAssetPane::_startAnimation() {
   if (!this->_animation.updateID) {
      this->_animation.updateID = this->startTimer(16);
      this->_animation.elapsed.restart();
   }
}
void DKTextureAssetPane::_stopAnimation() {
   this->killTimer(this->_animation.updateID);
   this->_animation.updateID = 0;
}

void DKTextureAssetPane::paintEvent(QPaintEvent* event) {
   QFrame::paintEvent(event);
   //
   QPainter painter(this);
   if (this->_render != Render::Loading) {
      this->_stopAnimation();
   }
   switch (this->_render) {
      case Render::Null:
         _drawNullSymbol(painter, this->contentsRect());
         break;
      case Render::Loading:
         _drawLoadingSpinner(painter, this->contentsRect());
         break;
      case Render::Asset:
         #if !defined(QT_DESIGNER_LIB)
         {
            auto image = this->_handle->image();
            auto cr    = this->contentsRect();
            auto ir    = image.rect();
            auto scale = std::min((qreal)cr.width() / ir.width(), (qreal)cr.height() / ir.height());
            ir.setSize(ir.size() * scale);
            ir.moveCenter(cr.center());
            painter.drawImage(ir, this->_handle->image());
         }
         #endif
         break;
   }
}
void DKTextureAssetPane::timerEvent(QTimerEvent* event) {
   if (event->timerId() == this->_animation.updateID)
      this->update();
}