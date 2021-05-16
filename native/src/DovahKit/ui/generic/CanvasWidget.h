#pragma once
#include <shared_mutex>
#include <QWidget>

class CanvasWidget;
class CanvasLayer;
class CanvasLayerData;

class CanvasWidget : public QWidget {
   Q_OBJECT;
   public:
      CanvasWidget(QWidget* parent = nullptr);
      ~CanvasWidget();
   protected:
      QVector<CanvasLayer*> _layers;
      QSize _size;

      virtual void paintEvent(QPaintEvent* event) override;
      virtual QSize sizeHint() const override;

   public:
      CanvasLayer* addLayer(CanvasLayerData* data = nullptr);
      QList<CanvasLayer*> layers() const noexcept;

      inline QSize imageSize() const noexcept { return this->_size; }
      inline int imageWidth() const noexcept { return this->size().width(); }
      inline int imageHeight() const noexcept { return this->size().height(); }
      void setImageSize(const QSize&) noexcept;
      void setImageSize(int w, int h) noexcept;
};

class CanvasLayer {
   friend class CanvasWidget;
   protected:
      CanvasWidget*    _owner = nullptr;
      CanvasLayerData* _data  = nullptr;
      bool   _visible = false;
      QPoint _pos;

      void _paint(QPainter&);

   public:
      inline CanvasWidget* owner() const noexcept { return this->_owner; }

      inline CanvasLayerData* data() const noexcept { return this->_data; }
      void setData(CanvasLayerData*);

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

class CanvasLayerData {
   friend class CanvasLayer;
   protected:
      mutable std::shared_mutex mutex;
      QList<CanvasLayer*> _users;
      QImage* _image = nullptr;

      void _paint(QPainter&, const QPoint& pos);

   public:
      QImage* image(); // doesn't lock
      void replaceWithImage(QImage*);

      QImage* checkOutImage();    // exclusively locks the mutex and grabs the image, for editing. you must check the image back in once your changes are made. undefined behavior if replacing data wholesale while checked out.
      void checkInImage(QImage*); // you must pass in the same image you checked out

      QRect rect() const noexcept;
      inline QList<CanvasLayer*> users() const noexcept { return this->_users; }
};