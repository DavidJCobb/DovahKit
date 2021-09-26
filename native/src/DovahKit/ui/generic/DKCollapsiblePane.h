#pragma once
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

class DKCollapsiblePane : public QFrame {
   Q_OBJECT;
   public:
      DKCollapsiblePane(QWidget* parent);

      inline bool collapsed() const noexcept { return !this->subwidgets.body->isVisible(); }
      inline QString title() const noexcept { return this->subwidgets.label->text(); }

   public slots:
      void setCollapsed(bool);
      inline void toggleCollapsed() { this->setCollapsed(!this->collapsed()); }

      void setTitle(const QString& t); // also sets the accessibleName

      // This is the "body" of the widget; you'd set a layout on it, append children to it, et cetera.
      inline QWidget* viewport() const noexcept { return this->subwidgets.body; }

   signals:
      void contentsCollapsed();
      void contentsExpanded();

   protected:
      struct _ToolbarEntry {
         QAction*     action = nullptr;
         QToolButton* widget = nullptr;
      };
      struct _Toolbar {
         QWidget* container = nullptr;
         QList<_ToolbarEntry> entries;

         _Toolbar(DKCollapsiblePane&);

         int indexOf(QAction*) const noexcept;

         void insertAction(QAction* subject, QAction* before = nullptr);
         void updateAction(QAction*);
         void removeAction(QAction*);
      };

      _Toolbar toolbar; // for QAction buttons shown between the panel title and the expand/collapse button
      struct {
         QFrame*      title  = nullptr; // panel header bar
         QLabel*      label  = nullptr; // panel title text
         QWidget*     body   = nullptr; // panel body
         QPushButton* toggle = nullptr; // expand/collapse button
      } subwidgets;
      struct {
         bool show_actions_when_collapsed = false;
      } state;

      void _updateToggle();
      void _updateToggle(bool state);

      virtual void actionEvent(QActionEvent* event) override;
};