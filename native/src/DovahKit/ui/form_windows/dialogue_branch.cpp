#include "./dialogue_branch.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "./dialogue_branch/DialogueBranchStartingTopicFormFilter.h"

FormDialogDialogueBranch::FormDialogDialogueBranch(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* picker = this->ui.startingTopic;
      auto* filter = this->_filters.starting_topic = new DialogueBranchStartingTopicFormFilter(this);
      picker->setAllowNone(false);
      picker->setAllowedFormType(dovah::form_type::topic);
      picker->setCustomFilter(filter);
   }

   this->load(); // this creates the working copy.
}
void FormDialogDialogueBranch::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_filters.starting_topic->setDialogueBranch(this->formStub());

   ui::bind(this->ui.editorID, this->editor_id());

   {
      auto flags = working.branch_flags;
      if (flags & loaded_form_type::branch_flag::blocking) {
         this->ui.type->setCurrentIndex(2);
      } else if (flags & loaded_form_type::branch_flag::top_level) {
         this->ui.type->setCurrentIndex(1);
      } else {
         this->ui.type->setCurrentIndex(0);
      }
   }
   QObject::connect(this->ui.type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      constexpr const auto all_type_flags = (
         loaded_form_type::branch_flag::blocking |
         loaded_form_type::branch_flag::top_level |
         loaded_form_type::branch_flag::normal
      );

      this->form->branch_flags &= ~all_type_flags;
      switch (i) {
         case 0:
            this->form->branch_flags |= loaded_form_type::branch_flag::normal;
            break;
         case 1:
            this->form->branch_flags |= loaded_form_type::branch_flag::top_level;
            break;
         case 2:
            this->form->branch_flags |= loaded_form_type::branch_flag::blocking;
            break;
      }
   });

   ui::bind(this->ui.flagExclusive, working.branch_flags, loaded_form_type::branch_flag::exclusive);
   ui::bind(this->ui.startingTopic, working.starting_topic, working);
}
void FormDialogDialogueBranch::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& working = *this->form;
}