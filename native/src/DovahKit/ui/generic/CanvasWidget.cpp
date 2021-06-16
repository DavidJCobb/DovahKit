#include "CanvasWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include "../../helpers/qt/ownership.h"

namespace {
   // Optimal format for QPainter's blend modes
   static constexpr QImage::Format INTERMEDIATE_IMAGE_FORMAT = QImage::Format_ARGB32_Premultiplied;

   QImage _drawAtop(QImage& src, QImage& dst, const QPoint& src_pos, QPainter::CompositionMode mode, qreal opacity = 1.0) {
      if (mode != QPainter::CompositionMode_SourceOver) {
         //
         // Qt's "multply" doesn't work like the "multiply" in image editors: it pays no heed to the 
         // destination alpha, effectively overwriting that with the source alpha. The only way to 
         // fix this is to apply the destination alpha to the source, and then apply the modified 
         // source (via multiply) to the destination.
         // 
         // The same is true for other modes, like "difference."
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
      painter.setOpacity(opacity);
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
   er = er.intersected(QRect(QPoint(0, 0), this->_size));
   //
   QPainter painter(this);
   painter.setClipRect(er);
   {
      auto prior = QImage(this->_size, INTERMEDIATE_IMAGE_FORMAT);
      prior.fill(Qt::GlobalColor::transparent);
      for (auto* child : this->layers()) {
         if (!child->visible())
            continue;
         if (child->isLayerGroup()) {
            auto* group = (CanvasWidgetLayerGroup*)child;
            //
            QImage after = group->render(this->_size, group->position());
            prior = _drawAtop(after, prior, { 0, 0 }, group->compositionMode(), group->opacity());
         } else {
            auto* layer = (CanvasWidgetLayer*)child;
            //
            QImage after = layer->render();
            prior = _drawAtop(after, prior, layer->position(), layer->compositionMode(), layer->opacity());
         }
      }
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
CanvasWidgetLayerGroup* CanvasWidget::createLayerGroup() {
   auto* layer = new CanvasWidgetLayerGroup(this);
   return layer;
}
QList<CanvasWidgetEntity*> CanvasWidget::layers() const noexcept {
   QList<CanvasWidgetEntity*> out;
   const auto& list = this->children();
   out.reserve(list.size());
   for (auto* child : list) {
      if (auto* l = qobject_cast<CanvasWidgetEntity*>(child))
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

void CanvasWidget::moveLayerBefore(CanvasWidgetEntity* subject, CanvasWidgetEntity* target) {
   cobb::qt::move_object_before(this, subject, target);
}
void CanvasWidget::moveLayerAfter(CanvasWidgetEntity* subject, CanvasWidgetEntity* target) {
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

#pragma region CanvasWidgetEntity
CanvasWidget* CanvasWidgetEntity::canvas() const noexcept {
   QObject* o = this->parent();
   do {
      if (auto* c = qobject_cast<CanvasWidget*>(o))
         return c;
   } while (o = o->parent());
   return nullptr;
}
QPoint CanvasWidgetEntity::effectivePosition() const noexcept {
   QPoint p = this->_pos;
   if (auto* container = this->parentLayer()) {
      p += container->effectivePosition();
   }
   return p;
}
      
void CanvasWidgetEntity::setPosition(const QPoint& to) noexcept {
   if (this->position() == to)
      return;
   //
   auto* c = this->canvas();
   if (c) {
      /*
      QRegion prior = this->region();
      this->_pos = to;
      //
      // Repaint the rects this layer used to occupy, and the rects it now occupies:
      //
      QRegion after = this->region();
      c->update(prior.united(after));
      */
      this->_pos = to;
      c->update();
   } else {
      this->_pos = to;
   }
}
void CanvasWidgetEntity::setPosition(int x, int y) noexcept {
   this->setPosition({ x, y });
}

void CanvasWidgetEntity::setOpacity(qreal v) noexcept {
   v = std::clamp(v, 0.0, 1.0);
   if (this->_opacity == v)
      return;
   this->_opacity = v;
   this->update();
}

void CanvasWidgetEntity::setVisible(bool v) noexcept {
   if (this->_visible == v)
      return;
   this->_visible = v;
   this->update();
}

void CanvasWidgetEntity::setCompositionMode(CompositionMode c) noexcept {
   if (this->_blendMode == c)
      return;
   this->_blendMode = c;
   this->update();
}

void CanvasWidgetEntity::update() {
   if (auto* c = this->canvas())
      c->update();
}
#pragma endregion

#pragma region CanvasWidgetLayer
CanvasWidgetLayer::~CanvasWidgetLayer() {
   this->setData(nullptr);
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

QImage CanvasWidgetLayer::render() {
   auto* data = this->data();
   if (!data)
      return QImage();
   auto  size = data->rect().size();
   auto  out  = QImage(size, INTERMEDIATE_IMAGE_FORMAT);
   {
      QPainter painter = QPainter(&out);
      out.fill(Qt::GlobalColor::transparent);
      data->paint(painter, QPoint(0, 0));
   }
   return out;
}
#pragma endregion

#pragma region CanvasWidgetLayerGroup
QList<CanvasWidgetEntity*> CanvasWidgetLayerGroup::childLayers() const noexcept {
   QList<CanvasWidgetEntity*> list;
   for (auto* child : this->children())
      if (auto* layer = qobject_cast<CanvasWidgetEntity*>(child))
         list.push_back(layer);
   return list;
}
CanvasWidgetLayer* CanvasWidgetLayerGroup::createLayer(CanvasWidgetLayerData* data) {
   auto* layer = new CanvasWidgetLayer(this);
   layer->setData(data);
   return layer;
}
CanvasWidgetLayerGroup* CanvasWidgetLayerGroup::createLayerGroup() {
   auto* layer = new CanvasWidgetLayerGroup(this);
   return layer;
}

QRegion CanvasWidgetLayerGroup::region() const noexcept {
   QRegion base;
   for (auto* child : this->children()) {
      auto* layer = qobject_cast<CanvasWidgetLayer*>(child);
      if (!layer)
         continue;
      base = base.united(layer->region());
   }
   return base;
}

void CanvasWidgetLayerGroup::moveLayerBefore(CanvasWidgetEntity* subject, CanvasWidgetEntity* target) {
   cobb::qt::move_object_before(this, subject, target);
}
void CanvasWidgetLayerGroup::moveLayerAfter(CanvasWidgetEntity* subject, CanvasWidgetEntity* target) {
   cobb::qt::move_object_after(this, subject, target);
}

QImage CanvasWidgetLayerGroup::render(QSize bounds, QPoint offset) {
   int w = bounds.width();
   int h = bounds.height();
   if (w <= 0 || h <= 0)
      return QImage();
   QImage out = QImage(w, h, INTERMEDIATE_IMAGE_FORMAT);
   {
      QPainter painter = QPainter(&out);
      out.fill(Qt::GlobalColor::transparent);
   }
   //
   for (auto* child : this->childLayers()) {
      if (!child->visible())
         continue;
      if (child->isLayerGroup()) {
         auto* group = (CanvasWidgetLayerGroup*)child;
         //
         QPoint c_offset = group->position() + offset;
         QImage source   = group->render(bounds, c_offset);
         out = _drawAtop(source, out, { 0, 0 }, child->compositionMode(), group->opacity());
      } else {
         auto* layer = (CanvasWidgetLayer*)child;
         //
         QImage source = layer->render();
         out = _drawAtop(source, out, layer->position(), layer->compositionMode(), layer->opacity());
      }
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
      this->content.cache_key = im.cacheKey();
   }
   painter.drawPixmap(pos, pm);
}
QRect CanvasWidgetLayerDataImage::rect() const noexcept {
   auto& im = this->content.image;
   if (im.isNull())
      return QRect();
   return im.rect();
}

QImage CanvasWidgetLayerDataImage::image() {
   return this->content.image;
}
void CanvasWidgetLayerDataImage::setImage(QImage i) {
   this->content.image = i;
   this->update();
}
#pragma endregion