#include "./material_object.h"
#include <limits>
#include "ui/utils/bind.h"
#include <QTextDocument> // Qt::mightBeRichText

FormDialogMaterialObject::FormDialogMaterialObject(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.model->setEnabled(false);
   {
      QString whats_this = this->ui.model->whatsThis();
      QString addendum   = tr(
         "DovahKit doesn't currently support saving NIF data back out into a file. "
         "This technical limitation makes it impossible to properly update a Material "
         "Object's property data, so we do not allow setting the model at this time."
      );
      if (whats_this.isEmpty()) {
         whats_this = addendum;
      } else {
         if (Qt::mightBeRichText(whats_this)) {
            whats_this += QString("<p>%1</p>").arg(addendum);
         } else {
            whats_this += QString("\n\n%1").arg(addendum);
         }
      }
      this->ui.model->setWhatsThis(whats_this);
   }

   {
      auto* widget = this->ui.flagExtSnow;
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, [widget, &editor]() {
         widget->setVisible(editor.get_current_game() == dovah::game::skyrim_special);
      });
      widget->setVisible(editor.get_current_game() == dovah::game::skyrim_special);
   }

   this->load(); // this creates the working copy.
}
void FormDialogMaterialObject::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.flagSinglePass, working.directional_material.flags, loaded_form_type::flag::single_pass);
   ui::bind(this->ui.flagExtSnow, working.directional_material.flags_ex, loaded_form_type::flag_ex::snow);
   ui::bind(this->ui.directionalX, working.directional_material.projection_vector.x);
   ui::bind(this->ui.directionalY, working.directional_material.projection_vector.y);
   ui::bind(this->ui.directionalZ, working.directional_material.projection_vector.z);
   ui::bind(this->ui.falloffScale, working.directional_material.falloff.scale);
   ui::bind(this->ui.falloffBias, working.directional_material.falloff.bias);
   ui::bind(this->ui.uvScaleNoise, working.directional_material.noise_uv_scale);
   ui::bind(this->ui.uvScaleMaterial, working.directional_material.material_uv_scale);
   ui::bind(this->ui.normalDampener, working.directional_material.normal_dampener);
   {
      auto* widget = this->ui.singlePassColor;
      auto& values = working.directional_material.single_pass_color;
      widget->setColor(QColor(
         values.r / 255.0F,
         values.g / 255.0F,
         values.b / 255.0F
      ));
      QObject::connect(widget, &DKColorPickerButton::colorChanged, this, [&values](QColor c) {
         values.r = c.redF();
         values.g = c.greenF();
         values.b = c.blueF();
      });
   }
   if (DovahKitCore::get().get_current_game() != dovah::game::skyrim_special) {
      this->ui.flagExtSnow->setVisible(false);
   }

   QObject::connect(this->ui.model, &DKFormNIFPicker::dataChanged, this, [this]() {
      //
      // TODO: When we implement the ability to save NIF data back out, we 
      //       need to write code here to update the form's NiProperties 
      //       based on the chosen model.
      //
   });
}
void FormDialogMaterialObject::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.model->commitTo(working.model, working);
}