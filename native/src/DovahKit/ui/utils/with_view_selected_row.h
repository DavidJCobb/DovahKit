#pragma once
#include <QAbstractItemView>
#include <QItemSelectionModel>

namespace ui {
   //
   // If the view has at least one row selected: retrieves the index of the 
   // first selected row, and invokes the passed-in callable with that index 
   // as an argument. Otherwise, does nothing.
   //
   template<typename ExecFunctor>
      requires requires (ExecFunctor&& ef, int row) {
         { ef(row) };
      }
   void with_view_selected_row(QAbstractItemView& view, ExecFunctor&& ef) {
      int row;
      {
         QItemSelectionModel* sm = view.selectionModel();
         if (!sm)
            return;
         auto sel = sm->selectedRows();
         if (sel.empty())
            return;
         row = sel[0].row();
      }
      ef(row);
   }

   //
   // If the view has at least one row selected: retrieves the index of the 
   // first selected row, and invokes the "exec" callable with that index as 
   // an argument. Otherwise, invokes the "no selection" callable.
   //
   template<typename ExecFunctor, typename NoSelectionFunctor>
      requires requires (ExecFunctor&& ef, int row, NoSelectionFunctor nsf) {
         { ef(row) };
         { nsf() };
      }
   void with_view_selected_row(QAbstractItemView& view, ExecFunctor&& ef, NoSelectionFunctor&& nsf) {
      int row;
      {
         QItemSelectionModel* sm = view.selectionModel();
         if (!sm) {
            nsf();
            return;
         }
         auto sel = sm->selectedRows();
         if (sel.empty()) {
            nsf();
            return;
         }
         row = sel[0].row();
      }
      ef(row);
   }
}