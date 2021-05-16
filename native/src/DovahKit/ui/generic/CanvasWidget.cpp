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

CanvasLayer* CanvasWidget::addLayer(CanvasLayerData* data) {
   auto* layer = new CanvasLayer;
   layer->_owner = this;
   this->_layers.push_back(layer);
   layer->setData(data);
   return layer;
}
QList<CanvasLayer*> CanvasWidget::layers() const noexcept {
   QList<CanvasLayer*> list;
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

#pragma region CanvasLayer
void CanvasLayer::_paint(QPainter& painter) {
   if (!this->_data)
      return;
   this->_data->_paint(painter, this->position());
}

void CanvasLayer::setData(CanvasLayerData* d) {
   if (d == this->_data)
      return;
   if (auto* old = this->_data)
      old->_users.removeOne(this);
   this->_data = d;
   if (d)
      d->_users.push_back(this);
}

void CanvasLayer::setPosition(const QPoint& to) noexcept {
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
void CanvasLayer::setPosition(int x, int y) noexcept {
   this->setPosition({ x, y });
}

QRect CanvasLayer::rect() const noexcept {
   if (!this->_data)
      return QRect();
   return this->_data->rect().translated(this->position());
}

void CanvasLayer::setVisible(bool v) noexcept {
   if (this->_visible == v)
      return;
   this->_visible = v;
   this->update();
}

void CanvasLayer::update() {
   if (!this->_data || !this->_owner)
      return;
   auto rect = this->data()->rect().translated(this->position());
   this->_owner->update(rect);
}
#pragma endregion

#pragma region CanvasLayerData
void CanvasLayerData::_paint(QPainter& painter, const QPoint& pos) {
   std::shared_lock guard(this->mutex);
   if (auto* image = this->image()) {
      painter.drawImage(pos, *image, image->rect(), Qt::NoOpaqueDetection);
      return;
   }
}
QRect CanvasLayerData::rect() const noexcept {
   std::shared_lock guard(this->mutex);
   if (!this->_image)
      return QRect();
   return this->_image->rect();
}

QImage* CanvasLayerData::image() {
   return this->_image;
}
void CanvasLayerData::replaceWithImage(QImage* input) {
   {
      std::unique_lock guard(this->mutex);
      this->_image = input;
   }
   for (auto* user : users())
      user->update();
}

QImage* CanvasLayerData::checkOutImage() {
   this->mutex.lock();
   return this->_image;
}
void CanvasLayerData::checkInImage(QImage* in) {
   assert(in == this->_image);
   this->mutex.unlock();
   for (auto* user : users())
      user->update();
}
#pragma endregion