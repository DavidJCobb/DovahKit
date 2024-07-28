#pragma once
#include <QTableView>

namespace ui {
   //
   // Corner Button:       Disabled
   // Drag/Drop Overwrite: Disabled, so that drag/drop inserts by default if you enable it at all
   // Selection Behavior:  Select Rows
   // Scroll Mode, H:      Pixels
   // Scroll Mode, V:      Rows
   // Vertical Header:     Hidden
   //
   extern void typical_tableview_config(QTableView*);
}