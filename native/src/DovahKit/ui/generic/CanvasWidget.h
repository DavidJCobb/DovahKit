#pragma once
#include <QPainter>
#include <QWidget>
#include "../../helpers/qt/traversal.h"

class CanvasWidget;
class CanvasWidgetEntity;
class CanvasWidgetLayer;
class CanvasWidgetLayerData;
class CanvasWidgetLayerGroup;

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
      CanvasWidgetLayerGroup* createLayerGroup();

      QList<CanvasWidgetEntity*> layers() const noexcept;

      inline QSize imageSize() const noexcept { return this->_size; }
      inline int imageWidth() const noexcept { return this->size().width(); }
      inline int imageHeight() const noexcept { return this->size().height(); }
      void setImageSize(const QSize&) noexcept;
      void setImageSize(int w, int h) noexcept;
      void setImageWidth(int w) noexcept;
      void setImageHeight(int h) noexcept;

      void moveLayerBefore(CanvasWidgetEntity* subject, CanvasWidgetEntity* target);
      void moveLayerAfter(CanvasWidgetEntity* subject, CanvasWidgetEntity* target);

      // Traverses all child and descendant layers, returning a list of those and any data 
      // objects that those may have. Useful if, say, you plan on using this widget in Lua 
      // and quickly need a way to quickly check whether the widget or any of its parts are 
      // referenced.
      QList<QObject*> allAssociatedObjects(bool includeWidgets = true) const noexcept;
};

// Base class for layers and groups
class CanvasWidgetEntity : public QObject {
   Q_OBJECT;
   friend class CanvasWidget;
   public:
      using CompositionMode = QPainter::CompositionMode;

   protected:
      const bool _isLayerGroup = false;
      bool   _visible = false;
      qreal  _opacity = 1.0;
      QPoint _pos;
      CompositionMode _blendMode = CompositionMode::CompositionMode_SourceOver;

   public:
      CanvasWidgetEntity(bool g, QObject* parent = nullptr) : QObject(parent), _isLayerGroup(g) {}

      inline bool isLayer() const noexcept { return !this->_isLayerGroup; }
      inline bool isLayerGroup() const noexcept { return this->_isLayerGroup; }
      
      CanvasWidget* canvas() const noexcept;
      inline CanvasWidgetEntity* parentLayer() const noexcept { return qobject_cast<CanvasWidgetEntity*>(this->parent()); }
      QPoint effectivePosition() const noexcept;
      
      inline QPoint position() const noexcept { return this->_pos; }
      inline int x() const noexcept { return this->position().x(); }
      inline int y() const noexcept { return this->position().y(); }
      void setPosition(const QPoint&) noexcept;
      void setPosition(int x, int y) noexcept;

      inline qreal opacity() const noexcept { return this->_opacity; }
      void setOpacity(qreal) noexcept;

      inline bool visible() const noexcept { return this->_visible; }
      void setVisible(bool) noexcept;

      inline CompositionMode compositionMode() const noexcept { return this->_blendMode; }
      void setCompositionMode(CompositionMode) noexcept;

      void update();
};

class CanvasWidgetLayer : public CanvasWidgetEntity {
   Q_OBJECT;
   friend class CanvasWidget;
   protected:
      CanvasWidgetLayerData* _data = nullptr;

   public:
      CanvasWidgetLayer(QObject* parent = nullptr) : CanvasWidgetEntity(false, parent) {}
      ~CanvasWidgetLayer();

      inline CanvasWidgetLayerData* data() const noexcept { return this->_data; }
      void setData(CanvasWidgetLayerData*);

      // Returns the region occupied by this layer and all of its descendants, recursing as needed. 
      // Note that this function doesn't take the layer's ancestor-layers (if any) into account; if 
      // called on a nested layer, it will not apply the ancestor-layers' position offsets.
      QRegion region() const noexcept;

      QImage render(QRect canvas, QPoint effective_position);
};

class CanvasWidgetLayerGroup : public CanvasWidgetEntity {
   Q_OBJECT;
   friend class CanvasWidget;
   public:
      CanvasWidgetLayerGroup(QObject* parent = nullptr) : CanvasWidgetEntity(true, parent) {}

      CanvasWidgetLayer* createLayer(CanvasWidgetLayerData* data = nullptr);
      CanvasWidgetLayerGroup* createLayerGroup();
      QList<CanvasWidgetEntity*> childLayers() const noexcept;

      // Returns the region occupied by this layer and all of its descendants, recursing as needed. 
      // Note that this function doesn't take the layer's ancestor-layers (if any) into account; if 
      // called on a nested layer, it will not apply the ancestor-layers' position offsets.
      QRegion region() const noexcept;

      void moveLayerBefore(CanvasWidgetEntity* subject, CanvasWidgetEntity* target);
      void moveLayerAfter(CanvasWidgetEntity* subject, CanvasWidgetEntity* target);

      QImage render(QRect canvas, QPoint effective_position);
};

class CanvasWidgetLayerData : public QObject {
   Q_OBJECT;
   friend class CanvasWidgetLayer;
   protected:
      QList<CanvasWidgetLayer*> _users;

      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept = 0;

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
      QImage content;

      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept override;

   public:
      using CanvasWidgetLayerData::CanvasWidgetLayerData; // inherit constructor

      QImage image();
      void setImage(QImage);

      virtual QRect rect() const noexcept override;
};