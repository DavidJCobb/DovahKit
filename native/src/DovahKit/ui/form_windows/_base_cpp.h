#pragma once
#include "_base.h"
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"

namespace form_dialog_helpers {
   template<class _dialog_t, typename loaded_form_t> void initialize(_dialog_t& dialog, dovah::form_stub* stub) {
      dialog.ui.setupUi(&dialog);
      //
      QObject::connect(dialog.ui.buttonCancel, &QPushButton::clicked, [&dialog]() {
         dialog.reject();
      });
      QObject::connect(dialog.ui.buttonOK, &QPushButton::clicked, [&dialog]() {
         dialog.save();
         dialog.accept();
      });
      //
      if constexpr (std::is_base_of_v<FormDialogBaseTemplate, _dialog_t>) {
         static_assert(!std::is_base_of_v<FormDialogWorkingCopyBase, _dialog_t>, "A form-edit dialog should not subclass both FormDialogBaseTemplate and FormDialogWorkingCopyBase!");
         //
         if (stub->formType == loaded_form_t::form_type) {
            dialog.stub = stub;
            dialog.form = stub->load().ptr_cast<loaded_form_t>();
         }
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
         auto _reload = [&dialog]() { dialog.form = dialog.stub->load().ptr_cast<loaded_form_t>(); };
         QObject::connect(&editor, &DovahKitCore::dataSaveComplete, &dialog, _reload);
         QObject::connect(&editor, &DovahKitCore::dataSaveFailed,   &dialog, _reload);
      }
   }
}