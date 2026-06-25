#pragma once
#include <QDialog>

namespace ui {
   //
   // Show the argument as a modal that blocks activation only of its immediate 
   // parent window, rather than the hierarchy all the way up to a top-level 
   // window.
   //
   extern void show_parent_scoped_modal(QDialog&);

   //
   // Equivalent to the above, but sets the dialog's parent in the very specific 
   // and poorly-described-in-the-documentation way required to avoid massive 
   // visual bugs (of the "your dialog is no longer a dialog, but instead just a 
   // normal child widget now") sort.
   //
   extern void show_parent_scoped_modal(QDialog&, QWidget* parent);
}