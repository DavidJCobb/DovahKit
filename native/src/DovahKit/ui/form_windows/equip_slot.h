#pragma once
#include "./_base.h"
#include "dovah/forms/EquipSlot.h"
#include "ui_equip_slot.h" // generated

class FormDialogEquipSlot :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::EquipSlot, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogEquipSlot(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogEquipSlot ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
