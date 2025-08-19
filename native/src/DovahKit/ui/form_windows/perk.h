#pragma once
#include "./_base.h"
#include "dovah/forms/Perk.h"
#include "ui_perk.h"

class PerkEntriesModel;

class FormDialogPerk :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Perk, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogPerk(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogPerk ui;
      struct {
         PerkEntriesModel* entries = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
