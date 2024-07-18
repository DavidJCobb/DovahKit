#pragma once
#include "./_base.h"
#include "dovah/forms/LeveledCharacter.h"
#include "ui_leveled_character.h" // generated
#include <QMenu>

class LeveledListModel;

class FormDialogLeveledCharacter :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LeveledCharacter, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   friend class LeveledListEditDialogHelpers;
   public:
      FormDialogLeveledCharacter(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLeveledCharacter ui;
      LeveledListModel* _model = nullptr;
      QMenu* _view_context = nullptr;
      
      void _overwrite_selected_leveled_object();

      virtual void _load_impl() override;
      virtual void _save_impl() override;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;
};
