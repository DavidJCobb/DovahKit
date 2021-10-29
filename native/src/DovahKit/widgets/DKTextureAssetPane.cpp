#include "DKTextureAssetPane.h"
#include <QPainter>
#include <QTimerEvent>
#if !defined(QT_DESIGNER_LIB)
   #include "../editor/asset_manager/asset_manager.h"
   #include "../editor/asset_manager/data/form_textureset.h"
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
      QObject::connect(&this->_handle, &DovahKitAssetReceptor::ready,    this, &DKTextureAssetPane::_onAssetHandled);
      QObject::connect(&this->_handle, &DovahKitAssetReceptor::failed,   this, &DKTextureAssetPane::_onAssetHandled);
      QObject::connect(&this->_handle, &DovahKitAssetReceptor::unloaded, this, &DKTextureAssetPane::_onAssetUnloaded);
   #endif
   //
   this->_throttle.timer.setSingleShot(true);
   QObject::connect(&this->_throttle.timer, &QTimer::timeout, this, [this]() {
      auto* s = this->_throttle.stub;
      auto  p = this->_throttle.path;
      this->_clearThrottleData();
      //
      this->_throttle.enabled = false; // HACK so we can use setAsset
      if (s) {
         this->setAsset(s);
      } else if (!p.isEmpty()) {
         this->setAsset(p);
      }
      this->_throttle.enabled = true;
   });
}

bool DKTextureAssetPane::hasAsset() const noexcept {
   #if defined(QT_DESIGNER_LIB)
      return false;
   #else
      if (this->_throttle.enabled) {
         if (this->_throttle.stub)
            return true;
         if (!this->_throttle.path.isEmpty())
            return true;
      }
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
   if (this->_handle != nullptr) {
      if (this->_handle->samePathAs(path))
         //
         // Setting a receptor to the asset it already holds won't re-emit a "ready" or "failed" 
         // signal if the asset is already ready or failed, so if we don't catch that case here, 
         // then we'll be stuck with a loading spinner that never ends.
         //
         return;
   }
   if (this->_throttle.enabled) {
      if (this->_throttle.path == path)
         return;
      this->_throttle.timer.start(this->_throttle.ms);
      this->_throttle.path = path;
      this->_throttle.stub = nullptr;
      this->_handle = nullptr;
      this->update();
      return;
   }
   this->_render = Render::Loading;
   #if !defined(QT_DESIGNER_LIB)
      auto& am = DovahKitAssetManager::get();
      this->_handle = std::move(am.requestAsset(path));
      this->update();
   #endif
}
void DKTextureAssetPane::setAsset(dovah::form_stub* stub) {
   if (!stub) {
      this->_render = Render::Null;
      #if !defined(QT_DESIGNER_LIB)
         this->_handle = nullptr;
      #endif
      this->update();
      return;
   }
   if (this->_handle != nullptr) {
      if (this->_handle->formStub() == stub)
         //
         // Setting a receptor to the asset it already holds won't re-emit a "ready" or "failed" 
         // signal if the asset is already ready or failed, so if we don't catch that case here, 
         // then we'll be stuck with a loading spinner that never ends.
         //
         return;
   }
   if (this->_throttle.enabled) {
      if (this->_throttle.stub == stub)
         return;
      this->_throttle.timer.start(this->_throttle.ms);
      this->_throttle.stub = stub;
      this->_throttle.path.clear();
      this->_handle = nullptr;
      this->update();
      return;
   }
   this->_render = Render::Loading;
   #if !defined(QT_DESIGNER_LIB)
      auto& am = DovahKitAssetManager::get();
      this->_handle = std::move(am.requestAsset(*stub));
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

void DKTextureAssetPane::setThrottleEnabled(bool e) {
   auto& t = this->_throttle;
   //
   t.enabled = e;
   if (!e) {
      t.timer.stop();
      //
      QString p;
      auto*   s = t.stub;
      std::swap(p, t.path);
      if (s) {
         this->setAsset(s);
      } else if (!p.isEmpty()) {
         this->setAsset(p);
      }
   }
}
void DKTextureAssetPane::setThrottleTime(uint ms) {
   auto& t = this->_throttle;
   //
   if (t.enabled && t.timer.isActive()) {
      auto rem    = t.timer.remainingTime();
      auto passed = t.ms - rem;
      t.ms = ms;
      if (passed > ms) {
         t.timer.start(passed - ms);
      } else {
         t.timer.start(0);
      }
   } else {
      t.ms = ms;
   }
}

void DKTextureAssetPane::_onAssetHandled() {
   #if !defined(QT_DESIGNER_LIB)
      DovahKitAssetReceptor* use = &this->_handle;
      if (this->_handle == nullptr || this->_handle.isFailed()) {
         this->_render = Render::Failed;
         this->_stopAnimation();
         this->update();
         return;
      }
      if (this->_handle->type() != DovahKitAsset::Type::DDS && !this->_handle->asTextureSet()) {
         this->_render = Render::Failed;
         this->_stopAnimation();
         this->update();
         return;
      }
      this->_render = Render::Asset;
      this->_stopAnimation();
      this->update();
   #else
      this->_render = Render::Failed;
      this->_stopAnimation();
      this->update();
   #endif
}
void DKTextureAssetPane::_onAssetUnloaded() {
   this->_render = Render::Null;
   this->_stopAnimation();
   this->update();
}

void DKTextureAssetPane::_clearThrottleData() {
   this->_throttle.stub = nullptr;
   this->_throttle.path.clear();
}
bool DKTextureAssetPane::_hasThrottleData() const noexcept {
   auto& t = this->_throttle;
   if (t.stub)
      return true;
   if (!t.path.isEmpty())
      return true;
   return false;
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
   p.restore();
}
void DKTextureAssetPane::_drawFailSymbol(QPainter& p, QRect rect) {
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
   p.drawLine( halfwidth, -halfwidth, -halfwidth,  halfwidth);
   p.drawLine(-halfwidth, -halfwidth,  halfwidth,  halfwidth);
   p.restore();
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
   if (this->_hasThrottleData()) {
      _drawLoadingSpinner(painter, this->contentsRect());
      return;
   }
   if (this->_render != Render::Loading) {
      this->_stopAnimation();
   }
   switch (this->_render) {
      case Render::Null:
         _drawNullSymbol(painter, this->contentsRect());
         break;
      case Render::Failed:
         _drawFailSymbol(painter, this->contentsRect());
         break;
      case Render::Loading:
         _drawLoadingSpinner(painter, this->contentsRect());
         break;
      case Render::Asset:
         #if !defined(QT_DESIGNER_LIB)
         {
            auto image = this->_handle->asQImage();
            if (image.isNull()) {
               if (auto* ts = this->_handle->asTextureSet()) {
                  auto& diffuse = ts->textures[0];
                  if (diffuse != nullptr)
                     image = diffuse->asQImage();
               }
               if (image.isNull()) {
                  break;
               }
            }
            auto cr    = this->contentsRect();
            auto ir    = image.rect();
            auto scale = std::min((qreal)cr.width() / ir.width(), (qreal)cr.height() / ir.height());
            ir.setSize(ir.size() * scale);
            ir.moveCenter(cr.center());
            painter.drawImage(ir, image);
         }
         #endif
         break;
   }
}
void DKTextureAssetPane::timerEvent(QTimerEvent* event) {
   if (event->timerId() == this->_animation.updateID)
      this->update();
}