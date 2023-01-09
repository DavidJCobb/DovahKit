#include "landtexture.h"
#include "./_base_cpp.h"
#include "helpers/bitwise.h"

#include "../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_form_dialogs, "The form-editing dialog for LandTextures is incomplete: the preview pane is not yet functional.");

FormDialogLandTexture::FormDialogLandTexture(dovah::form_stub* stub, QWidget* parent) : FormEditDialogBase(stub, parent) {
   form_dialog_helpers::initialize(*this, stub);
   //
   this->ui.texturesetPreview->setShowAlpha(false);
   this->ui.texturesetPreview->setThrottleEnabled(true);
   this->ui.texturesetPreview->setThrottleTime(75);
   //
   this->ui.havokMaterialType->setAllowedFormType(dovah::form_type::material_type);
   this->ui.textureset->setAllowedFormType(dovah::form_type::texture_set);
   QObject::connect(this->ui.textureset, &FormPicker::formChanged, this, [this](dovah::form_stub* stub) {
      this->ui.texturesetPreview->setAsset(stub);
   });
   //
   this->load();
}
void FormDialogLandTexture::_load_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->ui.editorID->setText(QString::fromStdString(this->stub->get_editor_id()));
   this->ui.havokMaterialType->setFormStub(this->form->havok.material.get_form_stub());
   this->ui.havokFriction->setValue(this->form->havok.friction);
   this->ui.havokRestitution->setValue(this->form->havok.restitution);
   this->ui.specularExponent->setValue(this->form->specular_exponent);
   this->ui.textureset->setFormStub(this->form->texture_set.get_form_stub());
   this->ui.grasses->pullStubs(this->form->grasses);
   //
   if (editor.get_current_game() == dovah::game::skyrim_classic) {
      this->ui.isSnow->setVisible(false);
      this->ui.isSnow->setEnabled(false);
   } else {
      this->ui.isSnow->setVisible(true);
      this->ui.isSnow->setEnabled(true);
      this->ui.isSnow->setChecked(this->form->remaster_flags & dovah::loaded_forms::LandTexture::remaster_flag::is_snow);
   }
   //
   if (auto* stub = this->form->texture_set.get_form_stub()) {
      this->ui.texturesetPreview->setAsset(stub);
   }
}
void FormDialogLandTexture::_save_impl() {
   auto& editor = DovahKitCore::get();
   //
   this->stub->editorID = this->ui.editorID->text().toStdString();
   this->save_form_id(this->form->havok.material, this->ui.havokMaterialType->formStub());
   this->form->havok.friction    = this->ui.havokFriction->value();
   this->form->havok.restitution = this->ui.havokRestitution->value();
   this->form->specular_exponent = this->ui.specularExponent->value();
   this->save_form_id(this->form->texture_set, this->ui.textureset->formStub());
   this->ui.grasses->commitStubs(this->form->grasses, *this->form);
   //
   if (editor.get_current_game() != dovah::game::skyrim_classic) {
      cobb::edit_bit(this->form->remaster_flags, dovah::loaded_forms::LandTexture::remaster_flag::is_snow, this->ui.isSnow->isChecked());
   }
}