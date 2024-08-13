#pragma once
#include "./_base.h"
#include <cassert>
#include <QDialog>
#include <QMetaMethod> // for wildcard QObject::disconnect
#include <QPushButton> // for buttonOK and buttonCancel handlers
#include "dovah/forms/components/extra_data/_templates.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#define CLASS_TEMPLATE_PARAMS template<typename LoadedFormType, bool UsesWorkingCopy>
#define CLASS_NAME FormEditDialogMixin<LoadedFormType, UsesWorkingCopy>

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::_unload_form_data() {
   if constexpr (uses_working_copy) {
      this->_destination_form = nullptr;
   } else {
      this->form = nullptr;
   }
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::_load_form_data() {
   if (!this->stub)
      return;
   if constexpr (uses_working_copy) {
      this->_destination_form = this->stub->load().ptr_cast<loaded_form_type>();
      if (!this->form) {
         this->form = (loaded_form_type*) this->stub->create_working_copy();
         assert(this->form != nullptr);
      }
   } else {
      this->form = this->stub->load().ptr_cast<loaded_form_type>();
   }
}

#pragma region Accessors
   CLASS_TEMPLATE_PARAMS
   std::string& CLASS_NAME::editor_id() {
      if constexpr (uses_working_copy) {
         return _stub_info.editor_id;
      } else {
         return this->stub->editorID;
      }
   }

   CLASS_TEMPLATE_PARAMS
   uint32_t CLASS_NAME::get_record_flags() const {
      if constexpr (uses_working_copy) {
         return _stub_info.record_flags;
      } else {
         return this->stub->get_record_flags();
      }
   }

   CLASS_TEMPLATE_PARAMS
   bool CLASS_NAME::test_record_flags(uint32_t bits) const {
      if constexpr (uses_working_copy) {
         return _stub_info.record_flags & bits;
      } else {
         return this->stub->test_record_flags(bits);
      }
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::edit_record_flags(uint32_t bits, bool clear_or_set) {
      if constexpr (uses_working_copy) {
         if (clear_or_set)
            _stub_info.record_flags |= bits;
         else
            _stub_info.record_flags &= ~bits;
      } else {
         this->stub->edit_record_flags(bits, clear_or_set);
      }
   }
   
   CLASS_TEMPLATE_PARAMS
   uint32_t& CLASS_NAME::record_flags()
      #ifndef __INTELLISENSE__
      requires (uses_working_copy)
      #endif
   {
      return _stub_info.record_flags;
   }
#pragma endregion

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::initialize(this auto&& self, dovah::form_stub& stub) {
   using self_type = std::decay_t<decltype(self)>;
   
   if constexpr (self_type::form_type == dovah::form_type::reference) {
      assert(dovah::form_type_is_reference(stub.form_type));
   } else {
      assert(stub.form_type == self_type::form_type);
   }
   self.stub = &stub;
   
   self._load_form_data();
   
   auto* dialog = self.asDialog();
   assert(dialog != nullptr);
   QObject::connect(dialog, &QDialog::accepted, dialog, [&self]() {
      self.save();
   });
   if constexpr (uses_working_copy) {
      QObject::connect(dialog, &QDialog::rejected, dialog, [&self]() {
         auto& editor = DovahKitCore::get();
         //
         emit editor.formWorkingCopyDeleteImminent(self.stub);
         self._unload_form_data();
         self.form = nullptr;
         self.stub->delete_working_copy();
         emit editor.formWorkingCopyDeleteComplete(self.stub);
         self.stub = nullptr;
      });
   }
   
   #pragma region Handle editor events
   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, dialog, [&self, dialog]() {
         self._unload_form_data();
         self.stub = nullptr;
         dialog->reject();
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, dialog, [&self, dialog](dovah::form_stub* stub, bool just_being_flagged) {
         if (stub == self.stub) {
            self._unload_form_data();
            self.stub = nullptr;
            dialog->reject();
         }
      });
      
      QObject::connect(&editor, &DovahKitCore::dataSaveImminent, dialog, [&self]() {
         self._unload_form_data();
      });
      //
      auto _reload = [&self]() {
         if (!self.stub)
            return;
         self._load_form_data();
      };
      QObject::connect(&editor, &DovahKitCore::dataSaveComplete, dialog, _reload);
      QObject::connect(&editor, &DovahKitCore::dataSaveFailed,   dialog, _reload);
   }
   #pragma endregion
   
   #pragma region UI boilerplate
      self.ui.setupUi(dialog);
      
      QObject::connect(self.ui.buttonCancel, &QPushButton::clicked, dialog, &QDialog::reject);
      QObject::connect(self.ui.buttonOK,     &QPushButton::clicked, dialog, &QDialog::accept);
   #pragma endregion
}

CLASS_TEMPLATE_PARAMS
CLASS_NAME::~FormEditDialogMixin() {
   this->_unload_form_data();
   if (this->stub) {
      if constexpr (uses_working_copy) {
         this->stub->delete_working_copy();
      }
      this->stub = nullptr;
   }
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::load() {
   if (!this->stub)
      return;
   if constexpr (uses_working_copy) {
      this->_stub_info.editor_id    = this->stub->editorID;
      this->_stub_info.record_flags = this->stub->get_record_flags();
   }
   this->_load_impl();
}

CLASS_TEMPLATE_PARAMS
void CLASS_NAME::save() {
   if (!this->stub)
      return;
   
   auto& editor = DovahKitCore::get();
   {  // Unhook signals so we aren't notified about ourselves. If we try to access `this->form` 
      // in, say, a formModified signal handler, we'll choke and die on a null pointer. I'd 
      // rather not require every such signal in every such dialog to have to guard against 
      // that edge-case; the dialog should be considered "dead" once you click "OK" on it.
      //
      // WARNING: If other objects' responses to these signals cause signals of the same type 
      //          to be emitted, the dialog won't catch those! In general, you should assume 
      //          that once `save()` is called, your dialog is ABSOLUTELY BLOODY DONE doing 
      //          any custom behaviors it has.
      // 
      //          Sadly, Qt's API doesn't seem to have any way of saying, "Prevent this one 
      //          object from reacting to this one signal this one time, while still allowing 
      //          it to react to all other signals, including signals of this same type that 
      //          are emitted after the one occurrence we wish to block."
      //
      auto* this_object = dynamic_cast<QObject*>(this);
      assert(this_object != nullptr && "FormEditDialogMixin subclasses should be QObjects.");
      QObject::disconnect(&editor, &DovahKitCore::formWorkingCopyCommitImminent, this_object, nullptr);
      QObject::disconnect(&editor, &DovahKitCore::formModificationImminent,      this_object, nullptr);
      QObject::disconnect(&editor, &DovahKitCore::formModified,                  this_object, nullptr);
      QObject::disconnect(&editor, &DovahKitCore::formWorkingCopyCommitComplete, this_object, nullptr);
   }
   if constexpr (uses_working_copy) {
      emit editor.formWorkingCopyCommitImminent(this->stub);
   }
   emit editor.formModificationImminent(this->stub);
   this->stub->set_edited(true);
   if constexpr (uses_working_copy) {
      {  // There's no "replace record flags" function on `form_stub`, so use this ugly hack:
         auto flags = this->_stub_info.record_flags;
         this->stub->edit_record_flags(flags, true);
         this->stub->edit_record_flags(~flags, false);
      }
      this->stub->editorID = this->_stub_info.editor_id;
   }
   this->_save_impl();
   if constexpr (uses_working_copy) {
      this->stub->commit_working_copy();
      this->form = nullptr;
      emit editor.formWorkingCopyCommitComplete(this->stub);
   }
   emit editor.formModified(this->stub);
}

#pragma region Helpers
   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::write_form_ref(dovah::form_reference_t& dst, dovah::form_stub* value) {
      if constexpr (uses_working_copy) {
         dst.set(*this->form, value);
      } else {
         dst.set(*this->stub->form, value);
      }
   }

   CLASS_TEMPLATE_PARAMS
   template<dovah::loaded_forms::components::extra_data_type ExtraType>
   void CLASS_NAME::write_extra_form_ref(
      dovah::loaded_forms::components::extra_data_list& extra,
      dovah::form_stub* value,
      bool remove_if_empty
   ) {
      using dummy_t = dovah::loaded_forms::components::formID_extra_data<0, dovah::loaded_forms::components::extra_data_type::action>;
      
      if (stub || !remove_if_empty) {
         auto* data = extra.get_or_create_by_type(ExtraType);
         if (data)
            this->write_form_ref(((dummy_t*)data)->form, stub);
      } else {
         extra.remove_by_type(ExtraType);
      }
   }
#pragma endregion

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")
