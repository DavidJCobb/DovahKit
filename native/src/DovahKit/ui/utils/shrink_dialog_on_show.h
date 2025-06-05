#pragma once
#include <QDialog>

namespace ui {
   //
   // Qt can be kind of stupid about widget and dialog sizes sometimes... 
   // Things like listviews, tableviews, and custom widgets which wrap them 
   // can be given a default size that is too tall, stretching the dialog 
   // and any layouts that are adjacent to said widgets.
   // 
   // These functions exist as a cheap hack to auto-size the dialog to its 
   // minimum size the next time it's shown.
   //
   extern void shrink_dialog_on_show(QDialog&);
   extern void shrink_dialog_height_on_show(QDialog&);
}