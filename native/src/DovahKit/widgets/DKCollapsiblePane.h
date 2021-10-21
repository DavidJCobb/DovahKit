/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <QFrame>
#include <QLabel>
#include <QPushButton>

class DKCollapsiblePane : public QFrame {
   Q_OBJECT;
   Q_PROPERTY(bool    collapsed READ collapsed WRITE setCollapsed DESIGNABLE true);
   Q_PROPERTY(QString title     READ title     WRITE setTitle     DESIGNABLE true USER true);
   Q_PROPERTY(bool    showActionsWhenCollapsed READ showActionsWhenCollapsed WRITE setShowActionsWhenCollapsed DESIGNABLE true);
   public:
      DKCollapsiblePane(QWidget* parent);

      inline bool collapsed() const noexcept { return this->state.collapsed; }
      inline QString title() const noexcept { return this->state.title; }

      // This is the "body" of the widget; you'd set a layout on it, append children to it, et cetera.
      inline QWidget* viewport() const noexcept { return this->subwidgets.body; }
      void setViewport(QWidget*);
      QWidget* takeViewport(); // abandons ownership of the viewport, and returns it

      inline QWidget* titleWidget() const noexcept { return this->subwidgets.label; }
      void setTitleWidget(QWidget*);
      QWidget* takeTitleWidget(); // abandons ownership of the widget, and returns it

      inline bool showActionsWhenCollapsed() const noexcept { return this->state.show_actions_when_collapsed; }
      void setShowActionsWhenCollapsed(bool);

   public slots:
      void setCollapsed(bool);
      inline void toggleCollapsed() { this->setCollapsed(!this->collapsed()); }

      void setTitle(const QString& t); // also sets the accessibleName

   signals:
      void contentsCollapsed();
      void contentsExpanded();

   protected:
      struct _ToolbarEntry {
         QAction*     action = nullptr;
         QPushButton* widget = nullptr;
      };
      class _Toolbar {
         public:
            QWidget* container = nullptr;
            QList<_ToolbarEntry> entries;

         protected:
            void _synchronize(QPushButton*, QAction*);
            void _updateTabOrder();
         public:
            _Toolbar(DKCollapsiblePane&);

            int indexOf(QAction*) const noexcept;

            void insertAction(QAction* subject, QAction* before = nullptr);
            void updateAction(QAction*);
            void removeAction(QAction*);
      };

      _Toolbar toolbar; // for QAction buttons shown between the panel title and the expand/collapse button
      struct {
         QFrame*      title  = nullptr; // panel header bar
         QWidget*     label  = nullptr; // panel title text
         QWidget*     body   = nullptr; // panel body
         QPushButton* toggle = nullptr; // expand/collapse button
      } subwidgets;
      struct {
         bool collapsed = false;
         bool show_actions_when_collapsed = false;
         QString title;
      } state;

      void _updateToggle();
      void _updateToggle(bool state);

      virtual void actionEvent(QActionEvent* event) override;
};