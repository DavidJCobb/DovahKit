#include "repaint.h"
#include <QAbstractItemView>
#include <QComboBox>
#include <QScrollArea>

namespace cobb::qt {
   extern void update_hierarchy(QWidget* root) {
      root->update();
      auto desc = root->findChildren<QWidget*>();
      for (auto* d : desc) {
         d->update();
         if (auto* combobox = qobject_cast<QComboBox*>(d)) {
            if (auto* view = combobox->view())
               if (auto* viewport = view->viewport())
                  viewport->update();
         } else if (auto* scrollbox = qobject_cast<QScrollArea*>(d)) {
            if (auto* body = scrollbox->widget())
               update_hierarchy(body);
         }
      }
   }
}
