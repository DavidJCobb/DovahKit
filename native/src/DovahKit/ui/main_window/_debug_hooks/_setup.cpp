#include "_setup.h"
#include <QAction>
#include "../../../helpers/class_list.h"

#include "ui_collapsible_pane.h"

namespace DovahKitDebug {
   using all_features = cobb::class_list<
      features::ui_collapsible_pane//,
   >;

   template<typename T> struct _add_functor {
      static void execute(QMenu* menu, QWidget* from) {
         auto* action = new QAction(menu);
         action->setText(T::name);
         QObject::connect(action, &QAction::triggered, [from]() { T::execute(from->window()); });
         menu->addAction(action);
      }
   };

   extern void add_features_to_menu(QMenu* menu) {
      auto* p = menu->parentWidget();
      if (p)
         p = p->window();
      all_features::for_each_with_args<_add_functor>(menu, p);
   }
}