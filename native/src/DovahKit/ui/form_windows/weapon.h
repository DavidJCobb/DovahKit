#pragma once
#include "./_base.h"
#include "dovah/forms/Weapon.h"
#include "ui_weapon.h" // generated
class WeaponEnchantmentFormFilter;

class FormDialogWeapon :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Weapon, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWeapon(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogWeapon ui;
      struct {
         WeaponEnchantmentFormFilter* enchantment = nullptr;
      } _filters;
      struct {
         dovah::form_stub* normal = nullptr;
         dovah::form_stub* staves = nullptr;
      } last_selected_enchantment;

      void _pull_templatable_data_to_ui();
      void _update_from_template_form();

      void _on_enchantment_selected();
      void _on_weapon_type_changed();
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
