#include "./volumetric_lighting.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogVolumetricLighting::FormDialogVolumetricLighting(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->load(); // this creates the working copy.
}
void FormDialogVolumetricLighting::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.intensity, working.intensity);
   {
      auto* src = this->ui.customColor;
      auto& dst = working.custom_color.color;
      {
         QColor color;
         color.setRedF(dst.r);
         color.setGreenF(dst.g);
         color.setBlueF(dst.b);
         src->setColor(color);
      }
      QObject::connect(src, &DKColorPickerButton::colorChanged, this, [&dst](QColor c) {
         dst.r = c.redF();
         dst.g = c.greenF();
         dst.b = c.blueF();
      });
   }
   ui::bind(this->ui.customColorContrib, working.custom_color.contribution);
   ui::bind(this->ui.densityContrib, working.density.contribution);
   ui::bind(this->ui.densitySize, working.density.size);
   ui::bind(this->ui.densitySpeedFall, working.density.speeds.falling);
   ui::bind(this->ui.densitySpeedWind, working.density.speeds.wind);
   ui::bind(this->ui.phaseContrib, working.phase_function.contribution);
   ui::bind(this->ui.phaseScatter, working.phase_function.scattering);
   ui::bind(this->ui.sampleRepartitionRangeFactor, working.sampling_repartition.range_factor);
}
void FormDialogVolumetricLighting::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
}