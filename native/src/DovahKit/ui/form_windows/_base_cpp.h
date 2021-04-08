#pragma once
#include "_base.h"
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include <QPushButton>

namespace form_dialog_helpers {
   template<class _dialog_t, typename loaded_form_t> void initialize(_dialog_t& dialog, dovah::form_stub* stub) {
      static_assert(std::is_base_of_v<FormDialogBaseTemplate, _dialog_t>, "This helper is meant for FormDialogBaseTemplate only.");
      dialog.ui.setupUi(&dialog);
      //
      if (stub->formType == loaded_form_t::form_type) {
         dialog.stub = stub;
         dialog.form = stub->load().ptr_cast<loaded_form_t>();
      }
      //
      QObject::connect(dialog.ui.buttonCancel, &QPushButton::clicked, [&dialog]() {
         dialog.reject();
      });
      QObject::connect(dialog.ui.buttonOK, &QPushButton::clicked, [&dialog]() {
         dialog.save();
         dialog.accept();
      });
      //
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

   template<class _dialog_t> void initialize(_dialog_t& dialog) {
      static_assert(std::is_base_of_v<FormDialogWorkingCopyBase, _dialog_t>, "This helper is meant for FormDialogWorkingCopyBase only.");
      dialog.ui.setupUi(&dialog);
      //
      QObject::connect(dialog.ui.buttonCancel, &QPushButton::clicked, &dialog, &_dialog_t::reject);
      QObject::connect(dialog.ui.buttonOK,     &QPushButton::clicked, &dialog, &_dialog_t::accept);
   }
}