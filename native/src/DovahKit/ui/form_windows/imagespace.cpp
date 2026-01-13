#include "./imagespace.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogImagespace::FormDialogImagespace(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.dofRadius->setRange(0, 7);
   ui::set_range<float>(this->ui.cineBrightness);
   ui::set_range<float>(this->ui.cineContrast);
   ui::set_range<float>(this->ui.cineSaturation);
   ui::set_range<float>(this->ui.cineTintAmount);
   ui::set_range<float>(this->ui.dofDistance);
   ui::set_range<float>(this->ui.dofRange);
   ui::set_range<float>(this->ui.dofStrength);
   ui::set_range<float>(this->ui.hdrBloomRadius);
   ui::set_range<float>(this->ui.hdrBloomRecv);
   ui::set_range<float>(this->ui.hdrBloomScale);
   ui::set_range<float>(this->ui.hdrBloomThreshold);
   ui::set_range<float>(this->ui.hdrEyeAdaptSpeed);
   ui::set_range<float>(this->ui.hdrEyeAdaptStrength);
   ui::set_range<float>(this->ui.hdrSkyScale);
   ui::set_range<float>(this->ui.hdrSunlightScale);
   ui::set_range<float>(this->ui.hdrWhite);

   this->load(); // this creates the working copy.
}
void FormDialogImagespace::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.cineBrightness,      working.cinematic.brightness);
   ui::bind(this->ui.cineContrast,        working.cinematic.contrast);
   ui::bind(this->ui.cineSaturation,      working.cinematic.saturation);
   ui::bind(this->ui.cineTintAmount,      working.tint.amount);
   {
      auto* widget = this->ui.cineTintColor;
      auto& value  = working.tint;
      widget->setColor(QColor::fromRgbF(value.r, value.g, value.b));
      QObject::connect(widget, &DKColorPickerButton::colorChanged, this, [this, &value](QColor color) {
         value.r = color.redF();
         value.g = color.greenF();
         value.b = color.blueF();
      });
   }
   ui::bind(this->ui.dofDistance,         working.depth_of_field.distance);
   ui::bind(this->ui.dofRange,            working.depth_of_field.range);
   ui::bind(this->ui.dofStrength,         working.depth_of_field.strength);
   ui::bind(this->ui.hdrBloomRadius,      working.hdr.bloom.blur_radius);
   ui::bind(this->ui.hdrBloomRecv,        working.hdr.bloom.receive_threshold);
   ui::bind(this->ui.hdrBloomScale,       working.hdr.bloom.scale);
   ui::bind(this->ui.hdrBloomThreshold,   working.hdr.bloom.threshold);
   ui::bind(this->ui.hdrEyeAdaptSpeed,    working.hdr.eye_adapt_speed);
   ui::bind(this->ui.hdrEyeAdaptStrength, working.hdr.eye_adapt_strength);
   ui::bind(this->ui.hdrSkyScale,         working.hdr.sky_scale);
   ui::bind(this->ui.hdrSunlightScale,    working.hdr.sunlight_scale);
   ui::bind(this->ui.hdrWhite,            working.hdr.white);

   {
      ui::bind(this->ui.dofFlagNoSky, working.depth_of_field.no_sky);
      //
      // Can't ui::bind to a bitfield, so...
      //
      this->ui.dofRadius->setValue(working.depth_of_field.radius);
      QObject::connect(this->ui.dofRadius, qOverload<int>(&QSpinBox::valueChanged), this, [this](int v) {
         this->form->depth_of_field.radius = v;
      });
   }
}
void FormDialogImagespace::_save_impl() {
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