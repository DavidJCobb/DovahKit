#pragma once
#include "./_base.h"
#include "dovah/forms/Message.h"
#include "ui_message.h"
class MessageButtonsModel;

class FormDialogMessage :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Message, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMessage(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMessage ui;
      struct {
         MessageButtonsModel* buttons = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      int _selected_button_row() const;
      void _push_button_conditions(int row);
};
