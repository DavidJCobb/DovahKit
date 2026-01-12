#pragma once
#include <cstdint>
#include <unordered_map>
#include <QDialog>
#include <QObject>
#include "helpers/singleton_ex.h"
namespace dovah {
   class form_stub;
   enum class form_type : uint8_t;
}
class FormEditDialogInterface;
class FormUseInfoDialog;
//
class CameraPathsDialog;
class IdleAnimationsDialog;
class RegionsDialog;

extern void open_edit_dialog_for_form(dovah::form_stub&, QWidget* parent);
extern void open_edit_dialog_for_form_type(dovah::form_type);
namespace dovahkit::subsystems::per_form_windows {
   //
   // Subsystem for managing per-form edit dialogs, per-form-type edit dialogs, and 
   // per-form use info dialogs.
   //
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      friend void ::open_edit_dialog_for_form(dovah::form_stub&, QWidget* parent);
      friend void ::open_edit_dialog_for_form_type(dovah::form_type);
      protected:
         core();
         ~core();

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

      protected:
         std::unordered_map<dovah::form_stub*, QDialog*> extant_form_edit_dialogs;
         std::unordered_map<dovah::form_stub*, QDialog*> extant_use_info_dialogs;
         struct {
            CameraPathsDialog*    camera_path = nullptr;
            IdleAnimationsDialog* idle        = nullptr;
            RegionsDialog*        region      = nullptr;
         } extant_form_type_dialogs;

      public:
         bool for_each_form_edit_dialog(std::function<bool(FormEditDialogInterface*)>);
         bool for_each_form_uses_dialog(std::function<bool(FormUseInfoDialog*)>);

         bool is_edit_dialog_open_for(const dovah::form_stub&) const noexcept;

         void show_form_type_edit_dialog(dovah::form_type);

         void show_use_info_dialog(dovah::form_stub&, QWidget* parent);
   };
};