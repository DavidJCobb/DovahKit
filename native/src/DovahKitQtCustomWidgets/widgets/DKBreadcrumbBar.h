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
   Q_PROPERTY(QChar textSeparatorAlt   READ textSeparatorAlt   WRITE setTextSeparatorAlt   DESIGNABLE true);
   Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity DESIGNABLE true);
   Q_PROPERTY(bool  typedPathsCanBeRelative READ typedPathsCanBeRelative WRITE setTypedPathsCanBeRelative DESIGNABLE true);
   public:
      DKBreadcrumbBar(QWidget* parent = nullptr);

      struct SegmentPalette {
         struct {
            QBrush fill;
            QPen   line;
            QPen   text;
         } main_button;
         struct {
            QBrush fill;
            QPen   line;
            QPen   icon;
         } menu_button;
      };

      struct Styles {
         Styles();

         struct {
            QMargins margins;
            struct {
               SegmentPalette normal;
               SegmentPalette hovered;
               SegmentPalette disabled;
            } colors;
            unsigned int menu_button_width; // includes borders
         } segment;
         unsigned int border_width;
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
            bool    has_menu = false;
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

      // If set, the breadcrumb menu will treat this item as the root: we 
      // will not show path segments outside of it, and paths will be 
      // treated as relative to it.
      QModelIndex forcedStem() const noexcept;
      void setForcedStem(const QModelIndex&);

      constexpr Qt::ItemDataRole segmentNameRole() const noexcept { return this->_data.name_role; }
      void setSegmentNameRole(Qt::ItemDataRole);

      constexpr bool textEditingAllowed() const noexcept;
      void setTextEditingAllowed(bool);

      // The primary separator for path segments, recognized when reading 
      // paths you set, and used when producing a path as output.
      constexpr QChar textSeparator() const noexcept;
      void setTextSeparator(QChar);

      // A secondary separator for path segments, recognized when reading 
      // paths you set.
      constexpr QChar textSeparatorAlt() const noexcept;
      void setTextSeparatorAlt(QChar);

      constexpr bool typedPathsCanBeRelative() const noexcept;
      void setTypedPathsCanBeRelative(bool);

      constexpr Qt::CaseSensitivity caseSensitivity() const noexcept;
      void setCaseSensitivity(Qt::CaseSensitivity);

      constexpr bool areAnySegmentsHidden() const noexcept;
      bool isEditingText() const noexcept;
      constexpr size_t segmentCount() const noexcept;
      constexpr size_t visibleSegmentCount() const noexcept;

   protected:
      bool _set_path_relative_to(QString, const QModelIndex& relative_to);
   public:
      QString path() const noexcept;
      bool setPath(QString); // returns a success bool
      bool setRelativePath(QString); // relative to current path

      QMenu* rootMenu() const noexcept;
      void setRootMenu(QMenu*); // does NOT take ownership

      constexpr const Styles& styles() const noexcept { return this->_styles; }
      void setStyles(const Styles&);

   protected:
      void _on_navigated();
      void _on_data_changed(const QModelIndex&);
      bool _on_before_item_deleted(const QModelIndex&);
      void _on_items_moved(const QModelIndex& src_parent, int first, int last, const QModelIndex& dst_parent);
      void _build_segment_menu(const QModelIndex& qpmi);
      void _re_layout(bool force = false);

      void _on_segment_hovered(size_t);

      static int _guesstimate_menu_text_x_offset(const QMenu&);
      QPoint _compute_menu_position(size_t segment_index) const;
      void _open_menu(size_t);
      void _start_menu_eavesdropping(QMenu&);
      bool _do_menu_eavesdropping(QMenu&, QEvent&); // returns true if the menu should NOT receive the event
      void _on_root_menu_hidden();
      void _on_segment_menu_item_selected(QAction*);
      void _on_segment_menu_hidden();
      void _close_any_open_menu();

      QAction* _segment_menu_action_by_qmi(const QModelIndex&);

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
         QPersistentModelIndex forced_stem;
         Qt::ItemDataRole name_role = Qt::ItemDataRole::DisplayRole;
      } _data;
      struct {
         QPointer<QMenu> menu;
         QRectF geometry;
      } _root_button;
      std::vector<segment> _segments;
      struct {
         bool  allowed   = true;
         QChar separator = '/';
         QChar separator_alt = '\\';
         bool can_be_relative = true;
         Qt::CaseSensitivity case_sensitivity = Qt::CaseSensitivity::CaseInsensitive;
      } _text_editing;
      struct {
         QMenu      segment_menu;
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