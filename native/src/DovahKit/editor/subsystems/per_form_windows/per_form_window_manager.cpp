#include "./per_form_window_manager.h"
#include "ui/form_windows/_base.h"
#include "ui/main_window/form_use_info.h"
#include "ui/main_window.h"
#pragma region Per-form-type dialogs
   #include "ui/form_group_windows/camera_path/CameraPathsDialog.h"
   #include "ui/form_group_windows/idle/IdleAnimationsDialog.h"
   #include "ui/form_group_windows/region/RegionsDialog.h"
#pragma endregion

namespace dovahkit::subsystems::per_form_windows {
   core::core() {
   }
   core::~core() {
   }

   bool core::for_each_form_edit_dialog(std::function<bool(FormEditDialogInterface*)> functor) {
      for (auto& pair : this->extant_form_edit_dialogs) {
         auto* dialog = pair.second;
         auto* casted = dynamic_cast<FormEditDialogInterface*>(dialog);
         if (casted)
            if ((functor)(casted))
               return true;
      }
      return false;
   }
   bool core::for_each_form_uses_dialog(std::function<bool(FormUseInfoDialog*)> functor) {
      for (auto& pair : this->extant_use_info_dialogs) {
         auto* dialog = pair.second;
         auto* casted = dynamic_cast<FormUseInfoDialog*>(dialog);
         if (casted)
            if ((functor)(casted))
               return true;
      }
      return false;
   }

   bool core::is_edit_dialog_open_for(const dovah::form_stub& stub) const noexcept {
      auto& extant = this->extant_form_edit_dialogs;
      auto  it     = extant.find(const_cast<dovah::form_stub*>(&stub));
      return it != extant.end();
   }

   void core::show_form_type_edit_dialog(dovah::form_type ft) {
      auto*    parent = &MainWindow::get();
      QDialog* result = nullptr;
      switch (ft) {
         case dovah::form_type::camera_path:
            result = this->extant_form_type_dialogs.camera_path;
            if (!result)
               result = this->extant_form_type_dialogs.camera_path = new CameraPathsDialog(parent);
            break;
         case dovah::form_type::idle:
            result = this->extant_form_type_dialogs.idle;
            if (!result)
               result = this->extant_form_type_dialogs.idle = new IdleAnimationsDialog(parent);
            break;
         case dovah::form_type::region:
            result = this->extant_form_type_dialogs.region;
            if (!result)
               result = this->extant_form_type_dialogs.region = new RegionsDialog(parent);
            break;
      }
      if (result) {
         result->show();
         result->raise();
         result->activateWindow();
      }
   }

   void core::show_use_info_dialog(dovah::form_stub& stub, QWidget* parent) {
      auto& extant = this->extant_use_info_dialogs;
      auto  it     = extant.find(&stub);
      if (it != extant.end()) {
         auto dialog = it->second;
         if (dialog) {
            dialog->raise();
            dialog->activateWindow();
            return;
         }
      }
      auto* dialog = new FormUseInfoDialog(&stub, parent);
      extant[&stub] = dialog;
      QObject::connect(dialog, &QDialog::finished, this, [this, &stub, dialog]() {
         auto& pfwins = DovahKitCore::get();
         auto& map    = this->extant_use_info_dialogs;
         auto  it = map.find(&stub);
         if (it != map.end())
            map.erase(it);
         dialog->deleteLater();
      });
      dialog->show();
   }
}