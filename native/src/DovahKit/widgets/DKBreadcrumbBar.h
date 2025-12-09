#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QMenu>
#include <QPainterPath>
#include <QPointer>
#include <QPushButton>
#include <QWidget>

class DKBreadcrumbBar : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool  textEditingAllowed READ textEditingAllowed WRITE setTextEditingAllowed DESIGNABLE true);
   Q_PROPERTY(QChar textSeparator      READ textSeparator      WRITE setTextSeparator      DESIGNABLE true);
   Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity DESIGNABLE true);
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
            QPen   icon = QPen(QColor(128, 128, 128), 1.5, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
         } menu_button;
      };

      struct Styles {
         struct {
            QMargins margins{ 5, 3, 5, 3 };
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
               SegmentPalette disabled{
                  .main_button = {
                     .fill = QColor(0, 0, 0, 0),
                     .line = QColor(0, 0, 0, 0),
                  },
                  .menu_button = {
                     .fill = QColor(0, 0, 0, 0),
                     .line = QColor(0, 0, 0, 0),
                     .icon = QPen(QColor(0, 0, 0), 1.5, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
                  },
               };
            } colors;
            unsigned int menu_button_width = 15; // includes borders
         } segment;
         unsigned int border_width = 1;
      };

   protected:
      enum class segment_paint_state {
         normal,
         hovered,
         disabled,
      };

      struct segment {
         public:
            bool culled = false;
            struct {
               QRectF main_button;
               QRectF menu_button;
               struct {
                  bool leading  = false;
                  bool trailing = false;
               } main_borders;
            } geometry;
            QString text;
            QMenu*  menu = nullptr;
            QPersistentModelIndex qmi;

         public:
            unsigned int width() const noexcept;
            void repaint(DKBreadcrumbBar&, QPainter&, segment_paint_state, size_t my_index) const;
      };

      static constexpr const size_t index_of_none        = (size_t)-1;
      static constexpr const size_t index_of_root_button = (size_t)-2;

   public:
      QAbstractItemModel* model() const noexcept;
      void setModel(QAbstractItemModel*);

      QModelIndex currentIndex() const noexcept;
      void setCurrentIndex(const QModelIndex&);

      constexpr bool textEditingAllowed() const noexcept;
      void setTextEditingAllowed(bool);

      constexpr QChar textSeparator() const noexcept;
      void setTextSeparator(QChar);

      constexpr Qt::CaseSensitivity caseSensitivity() const noexcept;
      void setCaseSensitivity(Qt::CaseSensitivity);

      constexpr bool areAnySegmentsHidden() const noexcept;
      bool isEditingText() const noexcept;
      constexpr size_t segmentCount() const noexcept;
      constexpr size_t visibleSegmentCount() const noexcept;

      QString path() const noexcept;
      bool setPath(QString); // returns a success bool

      QMenu* rootMenu() const noexcept;
      void setRootMenu(QMenu*); // does NOT take ownership

      constexpr const Styles& styles() const noexcept { return this->_styles; }

   protected:
      void _on_navigated();
      void _on_data_changed(const QModelIndex&);
      bool _on_before_item_deleted(const QModelIndex&);
      void _set_up_menu(QMenu&, const QModelIndex& qpmi);
      void _re_layout(bool force = false);

      void _on_segment_hovered(size_t);

      static int _guesstimate_menu_text_x_offset(QMenu&);
      void _close_menu(size_t);
      void _open_menu(size_t);
      void _start_menu_eavesdropping(QMenu&);
      bool _do_menu_eavesdropping(QMenu&, QEvent&); // returns true if the menu should NOT receive the event
      void _on_segment_menu_hidden();

      void _on_segment_clicked(const segment&);
      void _on_horizontal_arrow_key(bool left);
      void _on_vertical_arrow_key(bool up);

      void _begin_text_editing();
      void _update_textbox_value();

      void _recache_icons();

   public:
      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;
      //
      #pragma region Events
         virtual void changeEvent(QEvent* event);
         virtual void focusOutEvent(QFocusEvent* event);
         virtual void keyPressEvent(QKeyEvent* event);
         virtual void leaveEvent(QEvent* event);
         virtual void mouseMoveEvent(QMouseEvent* event);
         virtual void mousePressEvent(QMouseEvent* event);
         virtual void paintEvent(QPaintEvent* event);
         virtual void resizeEvent(QResizeEvent* event);
         virtual void showEvent(QShowEvent* event);

         virtual bool eventFilter(QObject* watched, QEvent* event);
      #pragma endregion

   signals:
      void currentIndexChanged(const QModelIndex&) const;
      void currentPathChanged(QString) const;

   public slots:
      void beginTextEditing();
      void cancelTextEditing();
      void finishTextEditing();

   protected:
      Styles _styles;
      struct {
         QPointer<QAbstractItemModel> model;
         QPersistentModelIndex index;
      } _data;
      struct {
         QPointer<QMenu> menu;
         QRectF geometry;
      } _root_button;
      std::vector<segment> _segments;
      struct {
         bool  allowed   = true;
         QChar separator = '/';
         Qt::CaseSensitivity case_sensitivity = Qt::CaseSensitivity::CaseInsensitive;
      } _text_editing;
      struct {
         QLineEdit* textbox = nullptr;
      } _subwidgets;
      struct {
         size_t menu_open_for   = index_of_none;
         size_t hovered_segment = index_of_none;
         struct {
            size_t count_shown = 0;
         } last_layout;
         struct {
            bool segments_changed = false;
         } next_layout;
         struct {
            bool cached = false;
            QPainterPath chevron_base;
            QPainterPath chevron_open;
            QPainterPath chevron_more; // root button, regardless of state, if any segments are culled
         } icons;
         //
         // Vile hacks:
         //
         bool last_click_closed_our_menu = false;
         bool next_mouseleave_is_from_menu_opening = false;
      } _state;
};

#include "./DKBreadcrumbBar.inl"