#pragma once
#include <cstdint>
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_object_window.h"
namespace dovah {
   class form_stub;
}

class ObjectWindow : public QWidget {
   Q_OBJECT
   //
   public:
      ObjectWindow(QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   private:
      Ui::ObjectWindow ui;
      struct {
         struct {
            QMenu menu;
            struct {
               QAction* create_form   = nullptr;
               QAction* edit          = nullptr;
               QAction* duplicate     = nullptr;
               QAction* show_use_info = nullptr;
               QAction* renumber      = nullptr;
               QAction* delete_form   = nullptr;
               QAction* separator     = nullptr;
               QAction* recalc_bounds = nullptr;
            } actions;
         } form_table;
         struct {
            QMenu menu;
            struct {
               QAction* collapse_children = nullptr;
               QAction* expand_children = nullptr;
               QAction* collapse_descendants = nullptr;
               QAction* expand_descendants = nullptr;
            } actions;
         } tree;
      } context_menus;

   private:
      dovah::form_stub* _get_selected_form();

      void _create_form_in_current_category();
      void _selected_form_edit();
      void _selected_form_duplicate();
      void _selected_form_show_users();
      void _selected_form_recalc_bounds();
      void _selected_form_renumber();
      void _selected_form_delete();

      void _set_selected_category_contents_expanded(bool expanded, bool recurse);
};
