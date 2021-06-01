#pragma once
#include <shared_mutex>
#include <QWidget>

class CanvasWidget;
class CanvasWidgetLayer;
class CanvasWidgetLayerData;

class CanvasWidget : public QWidget {
   Q_OBJECT;
   public:
      CanvasWidget(QWidget* parent = nullptr);
      ~CanvasWidget();
   protected:
      QVector<CanvasWidgetLayer*> _layers;
      QSize _size;

      virtual void paintEvent(QPaintEvent* event) override;
      virtual QSize sizeHint() const override;

   public:
      CanvasWidgetLayer* addLayer(CanvasWidgetLayerData* data = nullptr);
      QList<CanvasWidgetLayer*> layers() const noexcept;

      inline QSize imageSize() const noexcept { return this->_size; }
      inline int imageWidth() const noexcept { return this->size().width(); }
      inline int imageHeight() const noexcept { return this->size().height(); }
      void setImageSize(const QSize&) noexcept;
      void setImageSize(int w, int h) noexcept;
};

// Owned by the parent CanvasWidget, and deleted when that object is destroyed.
class CanvasWidgetLayer {
   friend class CanvasWidget;
   protected:
      CanvasWidget*          _owner = nullptr;
      CanvasWidgetLayerData* _data  = nullptr;
      bool   _visible = false;
      QPoint _pos;

      void _paint(QPainter&);

   public:
      ~CanvasWidgetLayer();

      inline CanvasWidget* owner() const noexcept { return this->_owner; }

      inline CanvasWidgetLayerData* data() const noexcept { return this->_data; }
      void setData(CanvasWidgetLayerData*);

      inline QPoint position() const noexcept { return this->_pos; }
      inline int x() const noexcept { return this->position().x(); }
      inline int y() const noexcept { return this->position().y(); }
      void setPosition(const QPoint&) noexcept;
      void setPosition(int x, int y) noexcept;

      QRect rect() const noexcept;

      inline bool visible() const noexcept { return this->_visible; }
      void setVisible(bool) noexcept;

      void update();
};

class CanvasWidgetLayerData : public QObject {
   Q_OBJECT;
   friend class CanvasWidgetLayer;
   protected:
      QList<CanvasWidgetLayer*> _users;

      virtual void paint(QPainter&, const QPoint& pos) noexcept = 0;

   public:
      CanvasWidgetLayerData(QObject* parent = nullptr) : QObject(parent) {};

      virtual QRect rect() const noexcept = 0;

      inline QList<CanvasWidgetLayer*> users() const noexcept { return this->_users; }

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