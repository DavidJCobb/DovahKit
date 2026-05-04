#include "./footstep_set.h"
#include "ui/utils/bind.h"

FormDialogFootstepSet::FormDialogFootstepSet(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool only_if_flagged) {
      for (auto& list : this->_footsteps.sublists)
         std::erase(list, stub);
   });

   {
      auto* widget = this->ui.currentMovementState;
      widget->clear();
      widget->addItem(tr("Walking"), (int)loaded_form_type::footstep_type::walk);
      widget->addItem(tr("Running"), (int)loaded_form_type::footstep_type::run);
      widget->addItem(tr("Sprinting"), (int)loaded_form_type::footstep_type::sprint);
      widget->addItem(tr("Sneaking"), (int)loaded_form_type::footstep_type::sneak);
      widget->addItem(tr("Swimming"), (int)loaded_form_type::footstep_type::swim);
   }

   this->ui.footsteps->setAllowedFormTypes({ dovah::form_type::footstep });

   this->load(); // this creates the working copy.
}
void FormDialogFootstepSet::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());

   static_assert(
      std::tuple_size_v<decltype(this->_footsteps.sublists)> == std::tuple_size_v<decltype(working.footsteps.sublists)>
   );
   for (size_t i = 0; i < this->_footsteps.sublists.size(); ++i) {
      auto&  src_list = working.footsteps.sublists[i];
      auto&  dst_list = this->_footsteps.sublists[i];
      size_t src_size = src_list.size();
      dst_list.reserve(src_size);
      for (auto& use : src_list) {
         if (!use)
            continue;
         dst_list.push_back(use.get_form_stub());
      }
   }
   QObject::connect(this->ui.currentMovementState, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
      if (this->_last_shown_footstep_list != -1) {
         this->_push_footstep_list(this->_last_shown_footstep_list);
      }
      this->_pull_footstep_list();
   });
   this->_pull_footstep_list();
}
void FormDialogFootstepSet::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_push_footstep_list();

   for (size_t i = 0; i < this->_footsteps.sublists.size(); ++i) {
      const auto& src_list = this->_footsteps.sublists[i];
      auto&       dst_list = working.footsteps.sublists[i];
      size_t src_size = src_list.size();
      size_t dst_size = dst_list.size();
      if (dst_size < src_size)
         dst_list.resize(src_size);
      for (size_t i = 0; i < src_size; ++i) {
         dst_list[i].set(working, src_list[i]);
      }
      if (dst_size > src_size) {
         for (size_t i = src_size; i < dst_size; ++i)
            dst_list[i].set(working, nullptr);
         dst_list.resize(src_size);
      }
   }
}

void FormDialogFootstepSet::_pull_footstep_list(size_t which) {
   if (which == -1)
      which = this->ui.currentMovementState->currentData().toInt();
   if (which >= this->_footsteps.sublists.size())
      return;
   this->ui.footsteps->clear();
   for(auto* stub : this->_footsteps.sublists[which])
      this->ui.footsteps->addStub(stub);
   this->_last_shown_footstep_list = which;
}
void FormDialogFootstepSet::_push_footstep_list(size_t which) {
   if (which == -1)
      which = this->ui.currentMovementState->currentData().toInt();
   if (which >= this->_footsteps.sublists.size())
      return;
   this->_footsteps.sublists[which] = this->ui.footsteps->stubs().toStdVector();
}