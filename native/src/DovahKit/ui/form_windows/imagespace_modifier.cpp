#include "./imagespace_modifier.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"

namespace {
   constexpr const int slider_resolution = 100;
}

#pragma region X-macro definitions
// X-macro. Params:
//  - keyframe field name
//  - keyframe editing control name
#define FOR_EACH_ANIMATED_FLOAT(DO) \
   DO(blurs.basic.radius,               blurRadius) \
   DO(blurs.motion.strength,            motionBlur) \
   DO(blurs.radial.strength,            radialBlurStrength) \
   DO(blurs.radial.ramp_up,             radialBlurRampUp) \
   DO(blurs.radial.start,               radialBlurStart) \
   DO(blurs.radial.ramp_down.start,     radialBlurDownStart) \
   DO(blurs.radial.ramp_down.value,     radialBlurRampDown) \
   DO(cinematic.saturation.mult,        cineSaturationMult) \
   DO(cinematic.saturation.add,         cineSaturationAdd) \
   DO(cinematic.brightness.mult,        cineBrightnessMult) \
   DO(cinematic.brightness.add,         cineBrightnessAdd) \
   DO(cinematic.contrast.mult,          cineContrastMult) \
   DO(cinematic.contrast.add,           cineContrastAdd) \
   DO(depth_of_field.strength,          dofStrength) \
   DO(depth_of_field.distance,          dofDistance) \
   DO(depth_of_field.range,             dofRange) \
   DO(double_vision.strength,           doubleVision) \
   DO(hdr.bloom.blur_radius.mult,       hdrBloomBlurRadiusMult) \
   DO(hdr.bloom.blur_radius.add,        hdrBloomBlurRadiusAdd) \
   DO(hdr.bloom.scale.mult,             hdrBloomScaleMult) \
   DO(hdr.bloom.scale.add,              hdrBloomScaleAdd) \
   DO(hdr.bloom.threshold.mult,         hdrBloomThresholdMult) \
   DO(hdr.bloom.threshold.add,          hdrBloomThresholdAdd) \
   DO(hdr.eye_adapt_speed.mult,         hdrEyeAdaptSpeedMult) \
   DO(hdr.eye_adapt_speed.add,          hdrEyeAdaptSpeedAdd) \
   DO(hdr.target_luminescence.min.mult, hdrTargetLumMinMult) \
   DO(hdr.target_luminescence.min.add,  hdrTargetLumMinAdd) \
   DO(hdr.target_luminescence.max.mult, hdrTargetLumMaxMult) \
   DO(hdr.target_luminescence.max.add,  hdrTargetLumMaxAdd) \
   DO(hdr.sky_scale.mult,               hdrSkyScaleMult) \
   DO(hdr.sky_scale.add,                hdrSkyScaleAdd) \
   DO(hdr.sunlight_scale.mult,          hdrSunlightScaleMult) \
   DO(hdr.sunlight_scale.add,           hdrSunlightScaleAdd)

// X-macro. Params:
//  - keyframe field name
//  - color control name
//  - alpha control name
//  - reset control name
#define FOR_EACH_ANIMATED_COLOR(DO) \
   DO(colors.fade, cineFadeColor, cineFadeAmount, cineFadeReset) \
   DO(colors.tint, cineTintColor, cineTintAmount, cineTintReset)

#define FOR_EACH_ANIMATED_UNUSED_FLOAT(DO) \
   DO(cinematic.unused.mult) \
   DO(cinematic.unused.add) \
   DO(unknown[0].mult) \
   DO(unknown[0].add) \
   DO(unknown[1].mult) \
   DO(unknown[1].add) \
   DO(unknown[2].mult) \
   DO(unknown[2].add) \
   DO(unknown[3].mult) \
   DO(unknown[3].add) \
   DO(unknown[4].mult) \
   DO(unknown[4].add) \
   DO(unknown[5].mult) \
   DO(unknown[5].add) \
   DO(unknown[6].mult) \
   DO(unknown[6].add) \
   DO(unknown[7].mult) \
   DO(unknown[7].add) \
   DO(unknown[8].mult) \
   DO(unknown[8].add)
#pragma endregion

FormDialogImagespaceModifier::FormDialogImagespaceModifier(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   // "Show in render window" checkbox. Currently disabled because we can't 
   // render imagespaces at all, much less render their animations.
   this->ui.flagDisplay->setVisible(false);

   this->ui.seek->setRange(0, slider_resolution);
   QObject::connect(this->ui.duration, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
      this->_set_duration(v);
   });
   QObject::connect(this->ui.currentTime, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
      _set_current_timestamp(v);
   });
   QObject::connect(this->ui.seek, qOverload<int>(&QSlider::valueChanged), this, [this](int v) {
      float position = (float)v / slider_resolution;
      _set_current_position(position);
   });
   QObject::connect(this->ui.buttonToPrevKeyframe, &QPushButton::clicked, this, [this]() {
      auto* prev = this->keyframes.keyframe_before_position(this->last_position);
      if (prev)
         this->_set_current_position(prev->position);
   });
   QObject::connect(this->ui.buttonToNextKeyframe, &QPushButton::clicked, this, [this]() {
      auto* next = this->keyframes.keyframe_after_position(this->last_position);
      if (next)
         this->_set_current_position(next->position);
   });

   {
      auto* widget = this->ui.dofMode;
      widget->clear();
      widget->addItem(tr("Front"), 1);
      widget->addItem(tr("Back"), 2);
      widget->addItem(tr("Front/Back"), 3);
   }

   //
   // Set up reset buttons. As a cheap hack, we use the enable state of the 
   // reset button itself to tell whether the property is defined for the 
   // current keyframe.
   //
   auto _set_up_color = [this](
      DKColorPickerButton* diffuse,
      QDoubleSpinBox* alpha,
      QPushButton* reset_button
   ) {
      QObject::connect(diffuse, &DKColorPickerButton::colorChanged, reset_button, [reset_button]() {
         reset_button->setEnabled(true);
      });
      QObject::connect(alpha, qOverload<double>(&QDoubleSpinBox::valueChanged), reset_button, [reset_button]() {
         reset_button->setEnabled(true);
      });
      QObject::connect(reset_button, &QPushButton::clicked, reset_button, [reset_button]() {
         reset_button->setEnabled(false);
      });
   };
   auto _set_up_float = [this](QDoubleSpinBox* editor, QPushButton* reset_button) {
      QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), reset_button, [reset_button]() {
         reset_button->setEnabled(true);
      });
      QObject::connect(reset_button, &QPushButton::clicked, reset_button, [reset_button]() {
         reset_button->setEnabled(false);
      });
   };
   
   #define X(field, control) _set_up_float(this->ui.control, this->ui.control##Reset);
   FOR_EACH_ANIMATED_FLOAT(X)
   #undef X

   #define X(field, color, alpha, reset) _set_up_color(this->ui.color, this->ui.alpha, this->ui.reset);
   FOR_EACH_ANIMATED_COLOR(X)
   #undef X

   this->load(); // this creates the working copy.
}
void FormDialogImagespaceModifier::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.flagAnimated, working.flags, loaded_form_type::modifier_flag::animatable);
   ui::bind(this->ui.dofUseTarget, working.depth_of_field.use_target);
   {  // Mode
      auto* widget = this->ui.dofMode;
      int   value  = 0;
      if (working.depth_of_field.mode.front)
         value |= 1;
      if (working.depth_of_field.mode.back)
         value |= 2;

      auto i = widget->findData(value);
      widget->setCurrentIndex(i);
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [widget, &working]() {
         int  value = 0;
         auto data  = widget->currentData();
         if (data.isValid())
            value = data.toInt();
         working.depth_of_field.mode.front = (value & 1) != 0;
         working.depth_of_field.mode.back  = (value & 2) != 0;
      });
   }
   ui::bind(this->ui.dofNoSky, working.depth_of_field.no_sky);
   {  // DoF blur radius is a bitfield, so we can't ui::bind it directly
      auto* widget = this->ui.dofRadius;
      widget->setValue(working.depth_of_field.blur_radius);
      QObject::connect(widget, qOverload<int>(&QSpinBox::valueChanged), this, [&working](int v) {
         working.depth_of_field.blur_radius = v;
      });
   }

   ui::bind(this->ui.radialBlurCenterX, working.blurs.radial.center.x);
   ui::bind(this->ui.radialBlurCenterY, working.blurs.radial.center.y);
   ui::bind(this->ui.radialBlurUseTarget, working.blurs.radial.flags, loaded_form_type::radial_blur_flag::use_target);

   ui::bind(this->ui.duration, working.duration);

   this->keyframes.import_data(working);
   this->_load_keyframe(0);
   this->ui.buttonToPrevKeyframe->setEnabled(false);
   this->ui.buttonToNextKeyframe->setEnabled(this->keyframes.keyframe_after_position(0) != nullptr);
}
void FormDialogImagespaceModifier::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_save_keyframe(this->last_position);
   this->keyframes.export_data(working);
}

void FormDialogImagespaceModifier::_set_current_position(float position) {
   this->_save_keyframe(this->last_position);
   this->_load_keyframe(position);
   this->last_position = position;

   auto* prev = this->keyframes.keyframe_before_position(position);
   auto* next = this->keyframes.keyframe_after_position(position);
   this->ui.buttonToPrevKeyframe->setEnabled(prev != nullptr);
   this->ui.buttonToNextKeyframe->setEnabled(next != nullptr);

   const auto blockers = std::array{
      QSignalBlocker(this->ui.currentTime),
      QSignalBlocker(this->ui.seek),
   };
   this->ui.currentTime->setValue(position * this->keyframes.duration);
   this->ui.seek->setValue(position * slider_resolution);
}
void FormDialogImagespaceModifier::_set_current_timestamp(float timestamp) {
   this->_set_current_position(timestamp / this->keyframes.duration);
}
void FormDialogImagespaceModifier::_set_duration(float duration_after) {
   float duration_prior = this->keyframes.duration;
   float position_prior = this->ui.currentTime->value() / duration_prior;

   this->keyframes.duration = duration_after;

   const auto blocker = QSignalBlocker(this->ui.currentTime);
   this->ui.currentTime->setValue(position_prior * duration_after);
}

void FormDialogImagespaceModifier::_load_keyframe(float position) {
   const auto* defined_kf = this->keyframes.keyframe_at_position(position);

   auto kf = this->keyframes.get_computed_keyframe(position, false);

   auto _update_float_ui = [](
      float value,
      bool  is_present,
      QDoubleSpinBox* editor,
      QPushButton*    button
   ) {
      editor->setValue(value);
      button->setEnabled(is_present);
   };
   auto _update_color_ui = [](
      QColor value,
      bool is_present,
      DKColorPickerButton* diffuse,
      QDoubleSpinBox*      alpha,
      QPushButton* reset_button
   ) {
      diffuse->setColor(value.rgb());
      alpha->setValue(value.alphaF());
      reset_button->setEnabled(is_present);
   };

   #define PRESENT(field) defined_kf ? defined_kf->field.has_value() : false

   #define X(field, control) _update_float_ui(kf.field, PRESENT(field), this->ui.control, this->ui.control##Reset);
   FOR_EACH_ANIMATED_FLOAT(X)
   #undef X

   #define X(field, color, alpha, reset) _update_color_ui(kf.field, PRESENT(field), this->ui.color, this->ui.alpha, this->ui.reset);
   FOR_EACH_ANIMATED_COLOR(X)
   #undef X

   #undef PRESENT
}
void FormDialogImagespaceModifier::_save_keyframe(float position) {
   //
   // Exit if there are no changed properties.
   //
   bool any_properties_defined = [this]() -> bool {
      #define X(field, control) if (this->ui.control##Reset->isEnabled()) return true;
      FOR_EACH_ANIMATED_FLOAT(X)
      #undef X

      #define X(field, color, alpha, reset) if (this->ui.reset->isEnabled()) return true;
      FOR_EACH_ANIMATED_COLOR(X)
      #undef X

      return false;
   }();
   if (!any_properties_defined)
      return;

   auto& kf = this->keyframes.get_or_create_keyframe(position);
   
   auto _read_float_ui = [](QDoubleSpinBox* editor, QPushButton* reset_button) -> std::optional<float> {
      if (reset_button->isEnabled()) {
         return editor->value();
      }
      return {};
   };
   auto _read_color_ui = [](
      DKColorPickerButton* diffuse,
      QDoubleSpinBox*      alpha,
      QPushButton* reset_button
   ) -> std::optional<QColor> {
      if (reset_button->isEnabled()) {
         auto color = diffuse->color();
         color.setAlphaF(alpha->value());
         return color;
      }
      return {};
   };
   

   #define X(field, control) kf.field = _read_float_ui(this->ui.control, this->ui.control##Reset);
   FOR_EACH_ANIMATED_FLOAT(X)
   #undef X

   #define X(field, color, alpha, reset) kf.field = _read_color_ui(this->ui.color, this->ui.alpha, this->ui.reset);
   FOR_EACH_ANIMATED_COLOR(X)
   #undef X
}