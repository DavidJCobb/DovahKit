#pragma once
#include <QAbstractItemView>

namespace ui {
   //
   // Enable dragging new items into the view to insert them, but don't enable 
   // overwriting existing items by dropping onto them.
   //
   // This, in itself, is not sufficient to add drag-and-drop support; your 
   // view's model must handle drag-and-drop properly as well. This includes 
   // responding to the virtual functions for drag-and-drop, but it also means 
   // you have to return `Qt::ItemFlag::ItemIsDropEnabled` when you're asked 
   // for the flags of an invalid QModelIndex.
   //
   extern void enable_inbound_drag_and_drop_insertions(QAbstractItemView*);
}