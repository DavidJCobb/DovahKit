#pragma once
#include "./_base.h"
#include "dovah/forms/AddOnNode.h"
#include "ui_addon_node.h" // generated

class FormDialogAddOnNode :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::AddOnNode, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogAddOnNode(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogAddOnNode ui;
      
      void _update_mps_flag();

      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
