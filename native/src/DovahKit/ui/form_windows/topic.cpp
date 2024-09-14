#include "./topic.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

#include <bitset>
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_stubs/helpers/for_each_dialogue_branch_topic.h"
#include "dovah/form_stubs/helpers/for_each_quest_topic.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"

FormDialogTopic::FormDialogTopic(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::topic)
         return;

      auto& list = this->_subtypes.sibling_topics;
      auto  it   = std::find(list.begin(), list.end(), stub);
      if (it == list.end()) {
         //
         // The topic that was modified wasn't a "sibling" of ours: that is, we don't 
         // have to avoid using its subtype. Let's verify that it isn't a *newly-added* 
         // sibling of ours.
         //
         auto* our_branch   = dovah::form_stub_helpers::get_dialogue_topic_branch(this->formStub());
         auto* their_branch = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
         if (our_branch != their_branch)
            return;
         if (!our_branch) {
            auto* our_quest   = dovah::form_stub_helpers::get_dialogue_topic_quest(this->formStub());
            auto* their_quest = dovah::form_stub_helpers::get_dialogue_topic_quest(stub);
            if (our_quest != their_quest)
               return;
         }
      }

      this->_update_available_subtypes();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::topic)
         return;

      auto& list = this->_subtypes.sibling_topics;
      auto  it   = std::find(list.begin(), list.end(), stub);
      if (it == list.end()) {
         //
         // The topic that was modified wasn't a "sibling" of ours: that is, we don't 
         // have to avoid using its subtype.
         //
         return;
      }

      this->_update_available_subtypes();
   });

   this->load(); // this creates the working copy.
}
void FormDialogTopic::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());

   QObject::connect(this->ui.subtype, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      auto data = this->ui.subtype->currentData();
      if (!data.isValid())
         return;

      size_t i = data.toInt();
      if (i >= dovah::dialogue::all_topic_subtypes.size())
         return;

      const auto& dfn = dovah::dialogue::all_topic_subtypes[i];
      this->form->subtype       = dfn.signature;
      this->form->data.category = dfn.category;
      this->form->data.subtype  = i;
   });
   {
      auto* dfn = dovah::dialogue::topic_subtype_by_signature(working.subtype);
      if (dfn) {
         this->_initial_category = dfn->category;
      }
      this->_update_available_subtypes();
      {
         auto* widget = this->ui.subtype;
         auto  i      = dfn ? dovah::dialogue::topic_subtype_index(*dfn) : (size_t)-1;
         auto  item_i = widget->findData(i);
         if (item_i < 0) {
            //
            // Unrecognized/invalid topic subtype. Reset the widget to a safe default, if one exists.
            //
            auto  cat  = this->_initial_category.value_or(dovah::dialogue::category::topic);
            auto* info = dovah::dialogue::default_subtype_for_category(cat);
            if (info) {
               i      = dovah::dialogue::topic_subtype_index(*info);
               item_i = widget->findData(i);
               if (item_i >= 0)
                  widget->setCurrentIndex(item_i);
            }
         } else {
            auto blocker = QSignalBlocker(widget);
            widget->setCurrentIndex(item_i);
         }
      }
   }

   ui::bind(this->ui.priority, working.priority);

   this->ui.text->setPlainText(editor.convert_localized_string(this->form->text));

   ui::bind(this->ui.flagDoAllBeforeRepeating, working.data.flags, loaded_form_type::dialogue_flag::do_all_before_repeating);
}
void FormDialogTopic::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   editor.assign_localized_string(this->form->text, this->ui.text->toPlainText());
}

/*static*/ QString FormDialogTopic::_subtype_name(size_t subtype_index) {
   auto& subtype = dovah::dialogue::all_topic_subtypes[subtype_index];
   return QString::fromLatin1(subtype.internal_name.data(), subtype.internal_name.size());
}

void FormDialogTopic::_update_available_subtypes() {
   std::bitset<dovah::dialogue::all_topic_subtypes.size()> subtypes;
   subtypes.set();

   dovah::form_stub* branch = nullptr;
   dovah::form_stub* quest  = nullptr;
   if (branch = dovah::form_stub_helpers::get_dialogue_topic_branch(this->formStub())) {
      quest = dovah::form_stub_helpers::get_dialogue_branch_quest(branch);
   } else {
      quest = dovah::form_stub_helpers::get_dialogue_topic_quest(this->formStub());
   }
   
   if (const auto& opt = this->_initial_category; opt.has_value()) {
      auto cat = opt.value();

      const auto& definitions = dovah::dialogue::all_topic_subtypes;
      for (size_t i = 0; i < definitions.size(); ++i) {
         auto& dfn = definitions[i];
         if (dfn.category != cat)
            subtypes.reset(i);
      }
   }
   
   this->_subtypes.sibling_topics.clear();
   auto _handle_other_topic = [this, &subtypes](dovah::form_stub* other) {
      if (other == this->formStub())
         return;
      if (other->is_deleted())
         return;
      auto loaded = other->load().ptr_cast<loaded_form_type>();
      if (!loaded)
         return;
      auto i = dovah::dialogue::topic_subtype_signature_to_index(loaded->subtype);
      if (i == (size_t)-1)
         return;
      this->_subtypes.sibling_topics.push_back(other);

      auto& dfn = dovah::dialogue::all_topic_subtypes[i];
      if (!dfn.is_reusable)
         subtypes.reset(i);
   };
   if (branch = dovah::form_stub_helpers::get_dialogue_topic_branch(this->formStub())) {
      dovah::form_stub_helpers::for_each_dialogue_branch_topic(branch, _handle_other_topic);
   } else {
      dovah::form_stub_helpers::for_each_quest_topic(quest, _handle_other_topic);
   }

   auto*  widget  = this->ui.subtype;
   auto   blocker = QSignalBlocker(widget);
   size_t prior   = widget->currentData().toInt();
   widget->clear();
   {
      const auto& definitions = dovah::dialogue::all_topic_subtypes;
      for (size_t i = 0; i < definitions.size(); ++i) {
         auto& dfn = definitions[i];
         if (!subtypes.test(i))
            continue;
         widget->addItem(_subtype_name(i), i);
      }
   }
   auto i = widget->findData(prior);
   if (i < 0) {
      widget->setCurrentIndex(0);
      emit widget->currentIndexChanged(i);
   } else
      widget->setCurrentIndex(i);
}