#pragma once
#include "./_base.h"
#include "dovah/forms/LeveledSpell.h"
#include "ui_leveled_spell.h" // generated
#include <QMenu>

class LeveledListModel;

class FormDialogLeveledSpell :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LeveledSpell, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   friend class LeveledListEditDialogHelpers;
   public:
      FormDialogLeveledSpell(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLeveledSpell ui;
      LeveledListModel* _model = nullptr;
      QMenu* _view_context = nullptr;
      
      void _overwrite_selected_leveled_object();

      virtual void _load_impl() override;
      virtual void _save_impl() override;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;
};
