#include "./relationship.h"
#include <limits>
#include "ui/utils/bind.h"

FormDialogRelationship::FormDialogRelationship(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.referrer->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.referent->setAllowedFormType(dovah::form_type::actor_base);
   {
      auto* widget = this->ui.relationship;
      for (size_t i = 0; i < widget->count(); ++i)
         widget->setItemData(i, i, Qt::ItemDataRole::UserRole);
   }
   this->ui.associationType->setAllowedFormType(dovah::form_type::association_type);

   this->load(); // this creates the working copy.
}
void FormDialogRelationship::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.referrer, working.referrer, working);
   ui::bind(this->ui.referent, working.referent, working);
   ui::bind(this->ui.relationship, working.rank);
   ui::bind(this->ui.associationType, working.association_type, working);
   {
      bool secret = this->record_flags() & loaded_form_type::form_flag::secret;
      if (!secret)
         secret = working.flags & loaded_form_type::relationship_flag::secret;

      this->ui.flagSecret->setChecked(secret);
      QObject::connect(this->ui.flagSecret, &QCheckBox::toggled, this, [this](bool checked) {
         if (checked) {
            this->record_flags() |= loaded_form_type::form_flag::secret;
            this->form->flags    |= loaded_form_type::relationship_flag::secret;
         } else {
            this->record_flags() &= ~loaded_form_type::form_flag::secret;
            this->form->flags    &= ~loaded_form_type::relationship_flag::secret;
         }
      });
   }
}
void FormDialogRelationship::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
}