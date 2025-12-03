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

   protected:
      struct segment {
         QPushButton* button = nullptr;
         QMenu*       menu   = nullptr;
         QPersistentModelIndex qmi;
      };

   public: // properties
      QAbstractItemModel* model() const noexcept;
      void setModel(QAbstractItemModel*);

      QModelIndex currentIndex() const noexcept;
      void setCurrentIndex(const QModelIndex&);

      bool allowTextEditing() const noexcept;

   protected:
      void _on_navigated();
      void _on_data_changed(const QModelIndex&);
      bool _on_before_item_deleted(const QModelIndex&);
      void _set_up_menu(QMenu&, const QModelIndex& qpmi);
      void _re_layout();

   public:
      #pragma region Events
         virtual void mousePressEvent(QMouseEvent* event);
         virtual void resizeEvent(QResizeEvent* event);
         virtual void showEvent(QShowEvent* event);
      #pragma endregion

   signals:
      void currentIndexChanged(const QModelIndex&) const;

   protected:
      struct {
         QPointer<QAbstractItemModel> model;
         QPersistentModelIndex index;
      } _data;
      struct {
         std::vector<segment> segments;
         QLineEdit* textbox = nullptr;
      } _subwidgets;
      struct {
         struct {
            bool   any_truncated = false;
            size_t count_shown   = 0;
         } last_layout;
         struct {
            bool segments_changed = false;
         } next_layout;
      } _state;
};