#pragma once
#include <QMenu>

namespace DovahKitDebug {
   // Add debugging menu items to the input menu, and set its visibility and enable state.
   // If the program is not compiled in Debug, the menu is hidden, disabled, and left empty.
   extern void add_features_to_menu(QMenu*);
}