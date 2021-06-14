#pragma once
#include <QWidget>
#include "../../helpers/qt/traversal.h"

class CanvasWidget;
class CanvasWidgetLayer;
class CanvasWidgetLayerData;

//
// A widget which displays CanvasWidgetLayer objects in its frame. The widget relies 
// on QObject's ownership-tree system: CWLs that use the widget as their parents will 
// be drawn, and will be destroyed when the widget is destroyed; and CWLs can be made 
// parents to each other in order to allow for "layer groups."
//
class CanvasWidget : public QWidget {
   Q_OBJECT;
   public:
      CanvasWidget(QWidget* parent = nullptr);
      ~CanvasWidget();
   protected:
      QSize _size;

      virtual void paintEvent(QPaintEvent* event) override;
      virtual QSize sizeHint() const override;

   public:

      // Add a layer to this canvas. The canvas becomes the parent object of the layer and 
      // so takes ownership of it.
      void addLayer(CanvasWidgetLayer* layer);

      CanvasWidgetLayer* createLayer(CanvasWidgetLayerData* data = nullptr);

      QList<CanvasWidgetLayer*> layers() const noexcept;

      inline QSize imageSize() const noexcept { return this->_size; }
      inline int imageWidth() const noexcept { return this->size().width(); }
      inline int imageHeight() const noexcept { return this->size().height(); }
      void setImageSize(const QSize&) noexcept;
      void setImageSize(int w, int h) noexcept;
      void setImageWidth(int w) noexcept;
      void setImageHeight(int h) noexcept;

      void moveLayerBefore(CanvasWidgetLayer* subject, CanvasWidgetLayer* target);
      void moveLayerAfter(CanvasWidgetLayer* subject, CanvasWidgetLayer* target);

      // Traverses all child and descendant layers, returning a list of those and any data 
      // objects that those may have. Useful if, say, you plan on using this widget in Lua 
      // and quickly need a way to quickly check whether the widget or any of its parts are 
      // referenced.
      QList<QObject*> allAssociatedObjects(bool includeWidgets = true) const noexcept;
};

class CanvasWidgetLayer : public QObject {
   Q_OBJECT;
   friend class CanvasWidget;
   protected:
      CanvasWidgetLayerData* _data = nullptr;
      bool   _visible = false;
      QPoint _pos;

      void _paint(QPainter&, QPoint p = QPoint(0, 0));

   public:
      using QObject::QObject; // inherit constructor
      ~CanvasWidgetLayer();

      CanvasWidget* canvas() const noexcept;
      inline CanvasWidgetLayer* parentLayer() const noexcept { return qobject_cast<CanvasWidgetLayer*>(this->parent()); }

      inline CanvasWidgetLayerData* data() const noexcept { return this->_data; }
      void setData(CanvasWidgetLayerData*);

      inline QPoint position() const noexcept { return this->_pos; }
      inline int x() const noexcept { return this->position().x(); }
      inline int y() const noexcept { return this->position().y(); }
      void setPosition(const QPoint&) noexcept;
      void setPosition(int x, int y) noexcept;

      QPoint effectivePosition() const noexcept;

      // Returns the region occupied by this layer and all of its descendants, recursing as needed. 
      // Note that this function doesn't take the layer's ancestor-layers (if any) into account; if 
      // called on a nested layer, it will not apply the ancestor-layers' position offsets.
      QRegion region() const noexcept;

      inline bool visible() const noexcept { return this->_visible; }
      void setVisible(bool) noexcept;

      void update();

      void moveLayerBefore(CanvasWidgetLayer* subject, CanvasWidgetLayer* target);
      void moveLayerAfter(CanvasWidgetLayer* subject, CanvasWidgetLayer* target);
};

class CanvasWidgetLayerData : public QObject {
   Q_OBJECT;
   friend class CanvasWidgetLayer;
   protected:
      QList<CanvasWidgetLayer*> _users;

      virtual void paint(QPainter&, const QPoint& pos) noexcept = 0;

   public:
      CanvasWidgetLayerData(QObject* parent = nullptr) : QObject(parent) {};
      virtual ~CanvasWidgetLayerData();

      virtual QRect rect() const noexcept = 0;

      inline QList<CanvasWidgetLayer*> users() const noexcept { return this->_users; }

      void update();

   signals:
      void attached(); // a previously-unused layer-data has been given to a layer
      void detached(); // a layer-data has ceased to be in use by any layers
};

class CanvasWidgetLayerDataImage : public CanvasWidgetLayerData {
   Q_OBJECT;
   protected:
      struct {
         QImage  image;
         QPixmap pixmap;
         qint64  cache_key = 0;
      } content;

      virtual void paint(QPainter&, const QPoint& pos) noexcept override;

   public:
      using CanvasWidgetLayerData::CanvasWidgetLayerData; // inherit constructor

      QImage& image();

      virtual QRect rect() const noexcept override;
};