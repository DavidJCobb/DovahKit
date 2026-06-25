#include "./show_parent_scoped_modal.h"

namespace ui {
   extern void show_parent_scoped_modal(QDialog& modal) {
      QWidget* parent_widget = modal.parentWidget();
      QWidget* parent_window = nullptr;
      if (parent_widget) {
         if (parent_widget->isWindow()) {
            parent_window = parent_widget;
         } else {
            parent_window = parent_widget->window();
         }
      }
      show_parent_scoped_modal(modal, parent_window);
   }
   
   extern void show_parent_scoped_modal(QDialog& modal, QWidget* parent) {
      auto* parent_window = parent;
      if (!parent_window->isWindow()) {
         parent_window = parent->window();
      }

      if (modal.isVisible()) {
         modal.reject();
      }
      modal.setWindowModality(Qt::WindowModality::WindowModal);
      modal.setResult(0);

      if (parent_window && !parent_window->isModal()) {
         //
         // So managing window modality in Qt is... kind of dumb. The three built-in 
         // options are:
         // 
         //  - Not a modal.
         // 
         //  - Modal to the entire containing window hierarchy, up to the top-level 
         //    window.
         // 
         //  - Modal to the entire application.
         // 
         // If we just want to be modal to the immediate parent, then the hack we have 
         // to use is to split that parent off from the rest of the window hierarchy.
         //
         auto* grandparent = qobject_cast<QWidget*>(parent_window->parent());
         if (grandparent) {
            QObject::connect(&modal, &QDialog::finished, parent_window, [parent_window, grandparent]() {
               parent_window->setParent(grandparent, parent_window->windowFlags());
            });
            parent_window->setParent(nullptr, parent_window->windowFlags());
            parent_window->show();
            modal.setParent(parent_window, modal.windowFlags());
         }
      }

      modal.open();
   }
}