#include "CanvasWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include "../../helpers/qt/ownership.h"

#pragma region CanvasWidget
CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
}
CanvasWidget::~CanvasWidget() {
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
   auto er = event->rect();
   //
   QPainter painter(this);
   for (auto* layer : this->layers()) {
      if (!layer->visible())
         continue;
      auto lr = layer->region();
      if (!lr.intersects(er))
         continue;
      layer->_paint(painter);
   }
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

void CanvasWidgetLayer::_paint(QPainter& painter, QPoint p) {
   QPoint effective = this->position() + p;
   if (auto* d = this->_data) {
      d->paint(painter, effective);
   }
   for (auto* child : this->children()) {
      auto* layer = qobject_cast<CanvasWidgetLayer*>(child);
      if (!layer)
         continue;
      layer->_paint(painter, effective);
   }
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

void CanvasWidgetLayer::update() {
   auto* c = this->canvas();
   if (!c)
      return;
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