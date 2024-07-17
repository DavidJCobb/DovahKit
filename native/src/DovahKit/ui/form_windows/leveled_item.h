#pragma once
#include "./_base.h"
#include "dovah/forms/LeveledItem.h"
#include "ui_leveled_item.h" // generated
#include <QMenu>

class LeveledListModel;

class FormDialogLeveledItem :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LeveledItem, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLeveledItem(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLeveledItem ui;
      LeveledListModel* _model = nullptr;
      QMenu* _view_context = nullptr;
      
      void _overwrite_selected_leveled_object();

      virtual void _load_impl() override;
      virtual void _save_impl() override;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;
};
