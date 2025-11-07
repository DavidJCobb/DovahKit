#pragma once
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_idle.h" // generated

class IdleAnimationsDialog : public QDialog {
   Q_OBJECT;
   public:
      IdleAnimationsDialog(QWidget* parent = nullptr);

   protected:
      Ui::IdleAnimationsDialog ui;
      struct {
         QMenu* menu = nullptr;
         struct {
            struct {
               QAction* add_action_root = nullptr;
            } graph;
         } actions;
      } _context;

      QModelIndex _get_selected_row();

      void _context_add_action_root();
};