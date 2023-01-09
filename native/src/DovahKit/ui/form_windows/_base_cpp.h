#pragma once
#include "_base.h"
#include <QPushButton>
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace form_dialog_helpers {
   template<class Dialog> void initialize(Dialog& dialog, dovah::form_stub* stub) requires std::is_base_of_v<AbstractFormEditDialog, Dialog> {
      using loaded_form_type = Dialog::loaded_form_type;
      constexpr const dovah::form_type_t form_type = loaded_form_type::form_type;

      dialog.ui.setupUi(&dialog);
      
      if constexpr (std::is_base_of_v<FormEditDialogBase, Dialog>) {
         if (stub->formType == form_type) {
            dialog.stub = stub;
            dialog.form = stub->load().ptr_cast<loaded_form_type>();
         }
      }
      //
      // Common GUI events:
      //
      QObject::connect(dialog.ui.buttonCancel, &QPushButton::clicked, &dialog, &Dialog::reject);
      QObject::connect(dialog.ui.buttonOK,     &QPushButton::clicked, &dialog, &Dialog::accept);
      //
      // React to changes made elsewhere in the program:
      //
      if constexpr (std::is_base_of_v<FormEditDialogBase, Dialog>) {
         auto& editor = DovahKitCore::get();
         QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, &dialog, [&dialog]() {
            dialog.form = nullptr;
            dialog.stub = nullptr;
            dialog.reject();
         });
         QObject::connect(&editor, &DovahKitCore::dataSaveImminent, &dialog, [&dialog]() {
            dialog.form = nullptr;
         });
         QObject::connect(&editor, &DovahKitCore::formDeletionImminent, &dialog, [&dialog](dovah::form_stub* stub, bool just_being_flagged) {
            if (stub == dialog.stub) {
               dialog.form = nullptr;
               dialog.stub = nullptr;
               dialog.reject();
            }
         });
         auto _reload = [&dialog]() {
            if (!dialog.stub)
               return;
            dialog.form = dialog.stub->load().ptr_cast<loaded_form_type>();
         };
         QObject::connect(&editor, &DovahKitCore::dataSaveComplete, &dialog, _reload);
         QObject::connect(&editor, &DovahKitCore::dataSaveFailed,   &dialog, _reload);
      }
   }
}