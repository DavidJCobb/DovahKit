#include "CanvasWidget.h"
#include <QPaintEvent>
#include <QPainter>

#pragma region CanvasWidget
CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
}
CanvasWidget::~CanvasWidget() {
   for(auto* l : this->_layers)
      delete l;
   this->_layers.clear();
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
   auto er = event->rect();
   //
   QPainter painter(this);
   for (auto* layer : this->_layers) {
      if (!layer->visible())
         continue;
      auto lr = layer->rect();
      if (!lr.intersects(er))
         continue;
      layer->_paint(painter);
   }
}
QSize CanvasWidget::sizeHint() const {
   return this->imageSize();
}

CanvasWidgetLayer* CanvasWidget::addLayer(CanvasWidgetLayerData* data) {
   auto* layer = new CanvasWidgetLayer;
   layer->_owner = this;
   this->_layers.push_back(layer);
   layer->setData(data);
   return layer;
}
QList<CanvasWidgetLayer*> CanvasWidget::layers() const noexcept {
   QList<CanvasWidgetLayer*> list;
   list.reserve(this->_layers.size());
   for (auto* l : this->_layers)
      list.push_back(l);
   return list;
}
void CanvasWidget::setImageSize(const QSize& s) noexcept {
   this->_size = s;
   this->setMinimumSize(s);
   this->updateGeometry();
}
void CanvasWidget::setImageSize(int w, int h) noexcept {
   this->setImageSize(QSize(w, h));
}
#pragma endregion

#pragma region CanvasWidgetLayer
CanvasWidgetLayer::~CanvasWidgetLayer() {
   this->setData(nullptr);
}

void CanvasWidgetLayer::_paint(QPainter& painter) {
   if (!this->_data)
      return;
   this->_data->paint(painter, this->position());
}

void CanvasWidgetLayer::setData(CanvasWidgetLayerData* d) {
   if (d == this->_data)
      return;
   if (auto* old = this->_data) {
      auto& list = old->_users;
      list.removeOne(this);
      if (list.isEmpty())
         emit old->detached();
   }
   this->_data = d;
   if (d) {
      auto& list = d->_users;
      list.push_back(this);
      if (list.size() == 1)
         emit d->attached();
   }
}

void CanvasWidgetLayer::setPosition(const QPoint& to) noexcept {
   auto prior = this->position();
   if (prior == to)
      return;
   this->_pos = to;
   if (!this->_data || !this->_owner)
      return;
   //
   // Repaint the rect this layer used to occupy, and the rect it now occupies:
   //
   auto prior_r = this->_data->rect().translated(prior);
   this->_owner->update(prior_r);
   this->update();
}
void CanvasWidgetLayer::setPosition(int x, int y) noexcept {
   this->setPosition({ x, y });
}

QRect CanvasWidgetLayer::rect() const noexcept {
   if (!this->_data)
      return QRect();
   return this->_data->rect().translated(this->position());
}

void CanvasWidgetLayer::setVisible(bool v) noexcept {
   if (this->_visible == v)
      return;
   this->_visible = v;
   this->update();
}

void CanvasWidgetLayer::update() {
   if (!this->_data || !this->_owner)
      return;
   auto rect = this->data()->rect().translated(this->position());
   this->_owner->update(rect);
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