#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QFrame>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QWidget>

class DKBreadcrumbBar : public QFrame {
   Q_OBJECT;
   public:
      DKBreadcrumbBar(QWidget* parent = nullptr);

      struct SegmentPalette {
         struct {
            QBrush fill = QColor(255, 255, 255);
            QPen   line = QPen(QColor(224, 224, 224), 0);
            QPen   text = QPen(QColor(0, 0, 0), 0);
         } main_button;
         struct {
            QBrush fill = QColor(255, 255, 255);
            QPen   line = QPen(QColor(224, 224, 224), 0);
            QPen   icon = QPen(QColor(128, 128, 128), 2);
         } menu_button;
      };

      struct Styles {
         struct {
            QMargins margins{ 3, 3, 3, 3 };
            struct {
               SegmentPalette normal;
               SegmentPalette hovered = {
                  .main_button = {
                     .fill = QColor(229, 243, 255),
                     .line = QPen(QColor(204, 232, 255), 0),
                  },
                  .menu_button = {
                     .fill = QColor(229, 243, 255),
                     .line = QPen(QColor(204, 232, 255), 0),
                  },
               };
               SegmentPalette disabled;
            } colors;
            unsigned int menu_button_width = 16;
         } segment;
      };

   protected:
      struct segment {
         bool culled = false;
         struct {
            QRectF main_button;
            QRectF menu_button;
         } geometry;
         QString text;
         QMenu*  menu = nullptr;
         QPersistentModelIndex qmi;
      };

      static constexpr const size_t index_of_none = (size_t)-1;

   public: // properties
      QAbstractItemModel* model() const noexcept;
      void setModel(QAbstractItemModel*);

      QModelIndex currentIndex() const noexcept;
      void setCurrentIndex(const QModelIndex&);

      bool allowTextEditing() const noexcept;

      constexpr const Styles& styles() const noexcept { return this->_styles; }

   protected:
      void _on_navigated();
      void _on_data_changed(const QModelIndex&);
      bool _on_before_item_deleted(const QModelIndex&);
      void _set_up_menu(QMenu&, const QModelIndex& qpmi);
      void _re_layout();

      void _on_segment_hovered(size_t);

      void _close_menu(size_t);
      void _open_menu(size_t);
      void _start_menu_eavesdropping(QMenu&);
      bool _do_menu_eavesdropping(QMenu&, QEvent&); // returns true if the menu should NOT receive the event
      void _on_segment_menu_hidden();

      void _on_segment_clicked(const segment&);
      void _on_horizontal_arrow_key(bool left);
      void _on_vertical_arrow_key();

   public:
      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;
      //
      #pragma region Events
         virtual void keyPressEvent(QKeyEvent* event);
         virtual void mouseMoveEvent(QMouseEvent* event);
         virtual void mousePressEvent(QMouseEvent* event);
         virtual void paintEvent(QPaintEvent* event);
         virtual void resizeEvent(QResizeEvent* event);
         virtual void showEvent(QShowEvent* event);

         virtual bool eventFilter(QObject* watched, QEvent* event);
      #pragma endregion

   signals:
      void currentIndexChanged(const QModelIndex&) const;

   protected:
      Styles _styles;
      struct {
         QPointer<QAbstractItemModel> model;
         QPersistentModelIndex index;
      } _data;
      std::vector<segment> _segments;
      struct {
         QLineEdit* textbox = nullptr;
      } _subwidgets;
      struct {
         size_t menu_open_for   = index_of_none;
         size_t hovered_segment = index_of_none;
         struct {
            bool   any_truncated = false;
            size_t count_shown   = 0;
         } last_layout;
         struct {
            bool segments_changed = false;
         } next_layout;
      } _state;
};