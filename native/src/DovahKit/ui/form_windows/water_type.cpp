#include "./water_type.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/pair_slider_to_spinbox.h"
#include "ui/utils/set_range.h"

FormDialogWaterType::FormDialogWaterType(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.spell->setAllowedFormType(dovah::form_type::spell);
   this->ui.imagespace->setAllowedFormType(dovah::form_type::imagespace);
   this->ui.openSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.material->setAllowedFormType(dovah::form_type::material_type);

   #pragma push_macro("PAIR")
   #define PAIR(name) ui::pair_slider_to_spinbox(this->ui.name##Slider, this->ui.name##Spinbox)
   #pragma region Physics
      PAIR(opacity);
      PAIR(reflectivityAmount);
      PAIR(reflectionMagnitude);
      PAIR(refractionMagnitude);
      PAIR(fresnelAmount);
   #pragma endregion
   #pragma region Specular
      PAIR(sunSpecPower);
      PAIR(sunSpecMagnitude);
      PAIR(sunSparklePower);
      PAIR(sunSparkleMagnitude);
      PAIR(specRadius);
      PAIR(specBrightness);
      PAIR(specPower);
   #pragma endregion
   #pragma region Noise
      #pragma region Layer 1
         PAIR(noise1WindDirection);
         PAIR(noise1WindSpeed);
         PAIR(noise1AmpScale);
         PAIR(noise1UVScale);
      #pragma endregion
      #pragma region Layer 2
         PAIR(noise2WindDirection);
         PAIR(noise2WindSpeed);
         PAIR(noise2AmpScale);
         PAIR(noise2UVScale);
      #pragma endregion
      #pragma region Layer 3
         PAIR(noise3WindDirection);
         PAIR(noise3WindSpeed);
         PAIR(noise3AmpScale);
         PAIR(noise3UVScale);
      #pragma endregion
      PAIR(noiseFalloff);
   #pragma endregion
   #pragma region Fog
      PAIR(fogAboveWaterAmount);
      PAIR(fogAboveWaterDistNear);
      PAIR(fogAboveWaterDistFar);
      PAIR(fogUnderwaterAmount);
      PAIR(fogUnderwaterDistNear);
      PAIR(fogUnderwaterDistFar);
   #pragma endregion
   #pragma region Depth
      PAIR(depthReflections);
      PAIR(depthRefraction);
      PAIR(depthNormals);
      PAIR(depthSpecular);
   #pragma endregion
   #pragma pop_macro("PAIR")

   this->load(); // this creates the working copy.
}
void FormDialogWaterType::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.spell, working.spell_to_apply, working);
   ui::bind(this->ui.flagDoesDamagePerSec, working.flags, loaded_form_type::flag::causes_damage);
   ui::bind(this->ui.damagePerSec, working.damage_per_second);
   ui::bind(this->ui.imagespace, working.underwater_imagespace, working);
   #pragma region Colors
      ui::bind(this->ui.colorDeep, working.water.colors.deep);
      ui::bind(this->ui.colorReflection, working.water.colors.reflection);
      ui::bind(this->ui.colorShallow, working.water.colors.shallow);
   #pragma endregion
   #pragma region Physics
      ui::bind(this->ui.openSound, working.sound, working);
      ui::bind(this->ui.material, working.material_type, working);
      ui::bind(this->ui.opacitySpinbox, working.opacity);
      ui::bind(this->ui.reflectivityAmountSpinbox, working.water.reflectivity);
      ui::bind(this->ui.reflectionMagnitudeSpinbox, working.water.reflection_magnitude);
      ui::bind(this->ui.refractionMagnitudeSpinbox, working.water.refraction_magnitude);
      ui::bind(this->ui.fresnelAmountSpinbox, working.water.fresnel);
      #pragma region Displacement
         ui::bind(this->ui.displacementForce, working.displacement.force);
         ui::bind(this->ui.displacementVelocity, working.displacement.velocity);
         ui::bind(this->ui.displacementFalloff, working.displacement.falloff);
         ui::bind(this->ui.displacementDampener, working.displacement.dampen);
         ui::bind(this->ui.displacementStartingSize, working.displacement.starting_size);
      #pragma endregion
      #pragma region Linear velocity
         ui::bind(this->ui.linearVelocityX, working.velocity.linear.x);
         ui::bind(this->ui.linearVelocityY, working.velocity.linear.y);
         ui::bind(this->ui.linearVelocityZ, working.velocity.linear.z);
      #pragma endregion
      #pragma region Angular velocity
         ui::bind(this->ui.angularVelocityX, working.velocity.angular.x);
         ui::bind(this->ui.angularVelocityY, working.velocity.angular.y);
         ui::bind(this->ui.angularVelocityZ, working.velocity.angular.z);
      #pragma endregion
   #pragma endregion
   #pragma region Specular
      ui::bind(this->ui.sunSpecPowerSpinbox, working.specular.sun.power);
      ui::bind(this->ui.sunSpecMagnitudeSpinbox, working.specular.sun.specular_magnitude);
      {
         auto* spinbox = this->ui.sunSparklePowerSpinbox;
         auto* slider  = this->ui.sunSparklePowerSlider;
         QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), this, [this, spinbox, slider](int v) {
            const auto blocker = QSignalBlocker(slider);
            slider->setValue(v);
            this->form->specular.sun.sparkle_power = v;
         });
         QObject::connect(slider, qOverload<int>(&QSlider::valueChanged), this, [spinbox](int v) {
            spinbox->setValue(v);
         });
         spinbox->setValue(working.specular.sun.sparkle_power);
      }
      ui::bind(this->ui.sunSparkleMagnitudeSpinbox, working.specular.sun.sparkle_magnitude);
      ui::bind(this->ui.specRadiusSpinbox, working.specular.radius);
      ui::bind(this->ui.specBrightnessSpinbox, working.specular.brightness);
      ui::bind(this->ui.specPowerSpinbox, working.specular.power);
   #pragma endregion
   #pragma region Noise
      #pragma push_macro("BIND")
      #define BIND(n) \
         { \
            auto& layer = working.noise.layers[n - 1]; \
            ui::bind(this->ui.noise##n##WindDirectionSpinbox, layer.wind_direction); \
            ui::bind(this->ui.noise##n##WindSpeedSpinbox, layer.wind_speed); \
            ui::bind(this->ui.noise##n##AmpScaleSpinbox, layer.amplitude_scale); \
            ui::bind(this->ui.noise##n##UVScaleSpinbox, layer.uv_scale); \
            ui::bind(this->ui.noise##n##Texture, layer.texture); \
         }

      BIND(1);
      BIND(2);
      BIND(3);
      ui::bind(this->ui.noiseFalloffSpinbox, working.noise.falloff);
      #pragma pop_macro("BIND")
   #pragma endregion
   #pragma region Fog
      ui::bind(this->ui.fogAboveWaterAmountSpinbox, working.fog.above_water.amount);
      ui::bind(this->ui.fogAboveWaterDistNearSpinbox, working.fog.above_water.distance.near);
      ui::bind(this->ui.fogAboveWaterDistFarSpinbox, working.fog.above_water.distance.far);
      ui::bind(this->ui.fogUnderwaterAmountSpinbox, working.fog.under_water.amount);
      ui::bind(this->ui.fogUnderwaterDistNearSpinbox, working.fog.under_water.distance.near);
      ui::bind(this->ui.fogUnderwaterDistFarSpinbox, working.fog.under_water.distance.far);
   #pragma endregion
   #pragma region Depth
      ui::bind(this->ui.depthReflectionsSpinbox, working.depth.reflections);
      ui::bind(this->ui.depthRefractionSpinbox, working.depth.refraction);
      ui::bind(this->ui.depthNormalsSpinbox, working.depth.normals);
      ui::bind(this->ui.depthSpecularSpinbox, working.depth.specular);
   #pragma endregion
}
void FormDialogWaterType::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
}