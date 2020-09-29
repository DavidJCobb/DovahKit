#pragma once
#include <cstdint>
#include <QDialog>
#include "_base.h"
#include "../../dovah/forms/Cell.h"
#include "../../dovah/forms/Faction.h"
#include "../../dovah/forms/components/extra_data/ownership.h"
#include "../../dovah/forms/components/extra_data/rank.h"
#include "ui_cell.h"

class FormDialogCell : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogCell(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogCell ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Cell> form;
      struct {
         dovah::form_stub* form = nullptr;
         int32_t           rank = 0;
         dovah::loaded_form_ptr<dovah::loaded_forms::Faction> loaded_faction;
      } working_ownership;
      //
      void _update_ownership_widgets();
      void _update_rank_picker();
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
