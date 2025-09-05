#pragma once
#include "./_base.h"
#include "dovah/forms/FootstepSet.h"
#include "ui_footstep_set.h"

class FormDialogFootstepSet :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::FootstepSet, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogFootstepSet(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogFootstepSet ui;
      union _ {
         ~_() { sublists.~array(); }

         std::array<std::vector<dovah::form_stub*>, 5> sublists = {};
         struct {
            std::vector<dovah::form_stub*> walk;
            std::vector<dovah::form_stub*> run;
            std::vector<dovah::form_stub*> sprint;
            std::vector<dovah::form_stub*> sneak;
            std::vector<dovah::form_stub*> swim;
         };
      } _footsteps;
      size_t _last_shown_footstep_list = -1;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _pull_footstep_list(size_t which = -1);
      void _push_footstep_list(size_t which = -1);
};
