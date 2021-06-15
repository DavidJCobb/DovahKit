#include "CanvasWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include "../../helpers/qt/ownership.h"

namespace {
   // Optimal format for QPainter's blend modes
   static constexpr QImage::Format INTERMEDIATE_IMAGE_FORMAT = QImage::Format_ARGB32_Premultiplied;

   QImage _drawAtop(QImage& src, QImage& dst, const QPoint& src_pos, QPainter::CompositionMode mode) {
      if (mode == QPainter::CompositionMode_Multiply) {
         //
         // Qt's "multply" doesn't work like the "multiply" in image editors: it pays no heed to the 
         // destination alpha, effectively overwriting that with the source alpha. The only way to 
         // fix this is to apply the destination alpha to the source, and then apply the modified 
         // source (via multiply) to the destination.
         // 
         // Confusingly enough, since we're copying from the destination of the eventual multiply 
         // operation to the source, the two pixmaps' roles are reversed during the copy: (source) 
         // is the destination and (out), the source.
         //
         QPainter cda = QPainter(&src); // Copy Destination Alpha
         cda.setCompositionMode(QPainter::CompositionMode_DestinationIn);
         QRectF copy_to   = src.rect();
         QRectF copy_from = src.rect();
         copy_to.setTopLeft(QPoint(0, 0));
         copy_from.setTopLeft(src_pos);
         cda.drawImage(copy_to, dst, copy_from);
      }
      QPainter painter(&dst);
      painter.setCompositionMode(mode);
      painter.drawImage(src_pos, src);
      //
      return dst;
   }

}

#pragma region CanvasWidget
CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
}
CanvasWidget::~CanvasWidget() {
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
   auto er = event->rect();
   //er = er.intersected(QRect(QPoint(0, 0), this->_size));
   er = QRect(QPoint(0, 0), this->_size); // *sigh* 
   //
   QPainter painter(this);
   painter.setClipRect(er);
   {
      auto prior = QImage(this->_size, INTERMEDIATE_IMAGE_FORMAT);
      prior.fill(Qt::GlobalColor::transparent);
      for (auto* layer : this->layers()) {
         if (!layer->visible())
            continue;
         QPoint pos  = layer->position();
         QSize  size = this->_size;
         size.setWidth(size.width() - pos.x());
         size.setHeight(size.height() - pos.y());
         //
         QImage after = layer->render(size);
         prior = _drawAtop(after, prior, pos, layer->_blendMode);
      }
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // just in case, I guess
      painter.drawImage(prior.rect(), prior);
   }
   painter.setClipRect(QRect(), Qt::NoClip);
}
QSize CanvasWidget::sizeHint() const {
   return this->imageSize();
}

void CanvasWidget::addLayer(CanvasWidgetLayer* layer) {
   assert(layer);
   layer->setParent(this);
   this->update(layer->region());
}
CanvasWidgetLayer* CanvasWidget::createLayer(CanvasWidgetLayerData* data) {
   auto* layer = new CanvasWidgetLayer(this);
   layer->setData(data);
   return layer;
}
QList<CanvasWidgetLayer*> CanvasWidget::layers() const noexcept {
   QList<CanvasWidgetLayer*> out;
   const auto& list = this->children();
   out.reserve(list.size());
   for (auto* child : list) {
      if (auto* l = qobject_cast<CanvasWidgetLayer*>(child))
         out.push_back(l);
   }
   return out;
}
void CanvasWidget::setImageSize(const QSize& s) noexcept {
   this->_size = s;
   this->setMinimumSize(s);
   this->updateGeometry();
   this->update();
}
void CanvasWidget::setImageSize(int w, int h) noexcept {
   this->setImageSize(QSize(w, h));
}
void CanvasWidget::setImageWidth(int w) noexcept {
   this->_size.setWidth(w);
   this->setMinimumSize(this->_size);
   this->updateGeometry();
   this->update();
}
void CanvasWidget::setImageHeight(int h) noexcept {
   this->_size.setHeight(h);
   this->setMinimumSize(this->_size);
   this->updateGeometry();
   this->update();
}

void CanvasWidget::moveLayerBefore(CanvasWidgetLayer* subject, CanvasWidgetLayer* target) {
   cobb::qt::move_object_before(this, subject, target);
}
void CanvasWidget::moveLayerAfter(CanvasWidgetLayer* subject, CanvasWidgetLayer* target) {
   cobb::qt::move_object_after(this, subject, target);
}

QList<QObject*> CanvasWidget::allAssociatedObjects(bool includeWidgets) const noexcept {
   if (includeWidgets) {
      QList<QObject*> objects = this->findChildren<QObject*>();
      QList<QObject*> data;
      data.reserve(objects.size()); // children will typically be layers; layers will typically have data
      for (auto* object : objects) {
         if (object->isWidgetType())
            continue;
         if (auto* layer = qobject_cast<CanvasWidgetLayer*>(object))
            if (auto* d = layer->data())
               data.push_back(d);
      }
      objects.append(data);
      return objects;
   }
   //
   // If we're excluding child and descendant widgets:
   //
   QList<QObject*> objects = this->findChildren<QObject*>();
   QList<QObject*> wanted;
   wanted.reserve(objects.size() * 2); // children will typically be layers; layers will typically have data
   for (auto* object : objects) {
      if (object->isWidgetType())
         continue;
      wanted.push_back(object);
      if (auto* layer = qobject_cast<CanvasWidgetLayer*>(object))
         if (auto* data = layer->data())
            wanted.push_back(data);
   }
   return wanted;
}
#pragma endregion

#pragma region CanvasWidgetLayer
CanvasWidgetLayer::~CanvasWidgetLayer() {
   this->setData(nullptr);
}

CanvasWidget* CanvasWidgetLayer::canvas() const noexcept {
   QObject* o = this->parent();
   do {
      if (auto* c = qobject_cast<CanvasWidget*>(o))
         return c;
   } while (o = o->parent());
   return nullptr;
}

QList<CanvasWidgetLayer*> CanvasWidgetLayer::childLayers() const noexcept {
   QList<CanvasWidgetLayer*> list;
   for (auto* child : this->children())
      if (auto* layer = qobject_cast<CanvasWidgetLayer*>(child))
         list.push_back(layer);
   return list;
}
CanvasWidgetLayer* CanvasWidgetLayer::createLayer(CanvasWidgetLayerData* data) {
   auto* layer = new CanvasWidgetLayer(this);
   layer->setData(data);
   return layer;
}

void CanvasWidgetLayer::_paint(QPainter& painter, QPoint p) {
   auto pos = this->position();
   painter.translate(pos);
   //
   auto children = this->childLayers();
   if (children.isEmpty()) {
      painter.setCompositionMode(this->_blendMode);
      if (auto* d = this->_data) {
         d->paint(painter, QPoint(0, 0));
      }
   } else {
      int w = 0;
      int h = 0;
      if (painter.hasClipping()) {
         auto rect = painter.clipBoundingRect();
         w = rect.width();
         h = rect.height();
      } else {
         auto* device = painter.device();
         w = device->width();
         h = device->height();
      }
      if (w > 0 && h > 0) {
         QPixmap pixmap = QPixmap(w, h);
         pixmap.fill(Qt::GlobalColor::transparent);
         //
         QPainter pp = QPainter(&pixmap);
         pp.setWorldTransform(painter.worldTransform());
         if (auto* d = this->_data) {
            d->paint(pp, QPoint(0, 0));
         }
         for (auto* child : children) {
            child->_paint(pp);
         }
 //        static_assert(false, "The ''multiply'' blend mode is broken by design! It ignores alpha on the destination layer and colorizes even transparent areas. We need to do this manually.");
            // Possible workaround: two paint steps: <https://forum.qt.io/post/302457> (this won't work exactly because it assumes a solid-color mask...)
         painter.setCompositionMode(this->_blendMode);
         if (this->_blendMode == QPainter::CompositionMode_Multiply) {
            QPixmap dest_copy = QPixmap(w, h);



            painter.drawPixmap(QRect(0, 0, w, h), pixmap);
         } else {
            painter.drawPixmap(QRect(0, 0, w, h), pixmap);
         }
      }
   }
   //
   painter.translate(-pos);
}

void CanvasWidgetLayer::setData(CanvasWidgetLayerData* d) {
   if (d == this->_data)
      return;
   QRect prior_rect;
   QRect after_rect;
   auto* c   = this->canvas();
   auto* old = this->_data;
   if (old) {
      if (c)
         prior_rect = old->rect();
      auto& list = old->_users;
      list.removeOne(this);
      if (list.isEmpty())
         emit old->detached();
   }
   this->_data = d;
   if (d) {
      if (c)
         after_rect = d->rect();
      auto& list = d->_users;
      list.push_back(this);
      if (list.size() == 1)
         emit d->attached();
   }
   if (prior_rect.isValid() || after_rect.isValid()) {
      QRegion region;
      if (prior_rect.isValid())
         region = region.united(prior_rect);
      if (after_rect.isValid())
         region = region.united(after_rect);
      c->update(region);
   }
}

void CanvasWidgetLayer::setPosition(const QPoint& to) noexcept {
   if (this->position() == to)
      return;
   //
   auto* c = this->canvas();
   if (c) {
      QRegion prior = this->region();
      this->_pos = to;
      //
      // Repaint the rects this layer used to occupy, and the rects it now occupies:
      //
      QRegion after = this->region();
      c->update(prior.united(after));
   } else {
      this->_pos = to;
   }
}
void CanvasWidgetLayer::setPosition(int x, int y) noexcept {
   this->setPosition({ x, y });
}

void CanvasWidgetLayer::setOpacity(qreal v) noexcept {
   v = std::clamp(v, 0.0, 1.0);
   if (this->_opacity == v)
      return;
   this->_opacity = v;
   this->update();
}

QPoint CanvasWidgetLayer::effectivePosition() const noexcept {
   QPoint p = this->_pos;
   if (auto* container = this->parentLayer()) {
      p += container->effectivePosition();
   }
   return p;
}

QRegion CanvasWidgetLayer::region() const noexcept {
   QRegion base;
   if (this->_data)
      base = base.united(this->_data->rect());
   for (auto* child : this->children()) {
      auto* layer = qobject_cast<CanvasWidgetLayer*>(child);
      if (!layer)
         continue;
      base = base.united(layer->region());
   }
   return base;
}

void CanvasWidgetLayer::setVisible(bool v) noexcept {
   if (this->_visible == v)
      return;
   this->_visible = v;
   this->update();
}

void CanvasWidgetLayer::setCompositionMode(CompositionMode c) noexcept {
   if (this->_blendMode == c)
      return;
   this->_blendMode = c;
   this->update();
}

void CanvasWidgetLayer::update() {
   CanvasWidget* c = nullptr;
   {
      //
      // Updates need to propagate up. The highest ancestor with a non-normal blend mode 
      // needs to be the one that updates.
      //
      static constexpr auto normal_blend_mode = CompositionMode::CompositionMode_SourceOver;
      //
      CanvasWidgetLayer* highestBlend = this;
      QObject* o = this->parent();
      do {
         if (auto* l = qobject_cast<CanvasWidgetLayer*>(o)) {
            if (l->_blendMode != normal_blend_mode)
               highestBlend = l;
         } else if (c = qobject_cast<CanvasWidget*>(o)) {
            break;
         }
      } while (o = o->parent());
      if (!c)
         return;
      if (highestBlend != this) {
         highestBlend->update();
         return;
      }
   }
   auto r = this->region();
   if (r.isEmpty())
      return;
   c->update(r);
}

void CanvasWidgetLayer::moveLayerBefore(CanvasWidgetLayer* subject, CanvasWidgetLayer* target) {
   cobb::qt::move_object_before(this, subject, target);
}
void CanvasWidgetLayer::moveLayerAfter(CanvasWidgetLayer* subject, CanvasWidgetLayer* target) {
   cobb::qt::move_object_after(this, subject, target);
}

QImage CanvasWidgetLayer::render(QSize bounds) {
   if (!bounds.isValid()) {
      bounds = this->region().boundingRect().size();
   }
   int w = bounds.width();
   int h = bounds.height();
   if (w <= 0 || h <= 0)
      return QImage();
   QImage out = QImage(w, h, INTERMEDIATE_IMAGE_FORMAT);
   {
      QPainter painter = QPainter(&out);
      out.fill(Qt::GlobalColor::transparent);
      //
      if (auto* d = this->_data)
         d->paint(painter, QPoint(0, 0));
   }
   //
   for (auto* child : this->childLayers()) {
      if (!child->visible())
         continue;
      auto c_offset = child->position();
      auto c_bounds = bounds;
      c_bounds.setWidth(c_bounds.width() - c_offset.x());
      c_bounds.setHeight(c_bounds.height() - c_offset.y());
      if (!c_bounds.isValid())
         continue;
      //
      QImage source = child->render(c_bounds);
      out = _drawAtop(source, out, c_offset, child->_blendMode);
   }
   return out;
}
#pragma endregion

#pragma region CanvasWidgetLayerData
CanvasWidgetLayerData::~CanvasWidgetLayerData() {
   auto list = this->users();
   this->_users.clear();
   for (auto* layer : list)
      layer->setData(nullptr);
}
void CanvasWidgetLayerData::update() {
   for (auto* layer : this->users())
      layer->update();
}
#pragma endregion

#pragma region CanvasWidgetLayerDataImage
void CanvasWidgetLayerDataImage::paint(QPainter& painter, const QPoint& pos) noexcept {
   auto& im = this->content.image;
   auto& pm = this->content.pixmap;
   if (im.isNull())
      return;
   if (im.cacheKey() != this->content.cache_key)
      pm = QPixmap();
   if (pm.isNull()) {
      pm = QPixmap::fromImage(im);
   }
   painter.drawPixmap(pos, pm);
}
QRect CanvasWidgetLayerDataImage::rect() const noexcept {
   auto& im = this->content.image;
   if (im.isNull())
      return QRect();
   return im.rect();
}

QImage& CanvasWidgetLayerDataImage::image() {
   return this->content.image;
}
#pragma endregion