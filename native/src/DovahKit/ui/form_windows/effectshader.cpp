#include "./effectshader.h"
#include "editor/asset_manager/asset_manager.h"
#include "ui/utils/bind.h"
#include "ui/utils/shrink_dialog_on_show.h"

FormDialogEffectShader::FormDialogEffectShader(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::shrink_dialog_height_on_show(*this);

   this->ui.particleDebris->setAllowedFormType(dovah::form_type::debris);
   this->ui.soundAmbient->setAllowedFormType(dovah::form_type::sound_descriptor);

   for (auto* widget : std::array{
      this->ui.membraneBlendModeSrc,
      this->ui.membraneBlendModeDst,
      this->ui.particleBlendModeSrc,
      this->ui.particleBlendModeDst,
   }) {
      constexpr const char* disambig = "blend mode";
      using enumeration = loaded_form_type::blend_mode;

      widget->clear();
      widget->addItem(tr("Zero",                 disambig), (int)enumeration::zero);
      widget->addItem(tr("One",                  disambig), (int)enumeration::one);
      widget->addItem(tr("Source Color",         disambig), (int)enumeration::src_color);
      widget->addItem(tr("Source Inverse Color", disambig), (int)enumeration::src_inv_color);
      widget->addItem(tr("Source Alpha",         disambig), (int)enumeration::src_alpha);
      widget->addItem(tr("Source Inverse Alpha", disambig), (int)enumeration::src_inv_alpha);
      widget->addItem(tr("Dest Color",           disambig), (int)enumeration::dst_color);
      widget->addItem(tr("Dest Inverse Color",   disambig), (int)enumeration::dst_inv_color);
      widget->addItem(tr("Dest Alpha",           disambig), (int)enumeration::dst_alpha);
      widget->addItem(tr("Dest Inverse Alpha",   disambig), (int)enumeration::dst_inv_alpha);
      widget->addItem(tr("Source Alpha SAT",     disambig), (int)enumeration::src_alpha_sat);
   }
   for (auto* widget : std::array{
      this->ui.membraneBlendOp,
      this->ui.particleBlendOp,
   }) {
      constexpr const char* disambig = "blend operation";
      using enumeration = loaded_form_type::blend_operation;

      widget->clear();
      widget->addItem(tr("Add", disambig), (int)enumeration::add);
      widget->addItem(tr("Subtract", disambig), (int)enumeration::sub);
      widget->addItem(tr("Reverse Subtract", disambig), (int)enumeration::sub_reverse);
      widget->addItem(tr("Minimum", disambig), (int)enumeration::min);
      widget->addItem(tr("Maximum", disambig), (int)enumeration::max);
   }
   for (auto* widget : std::array{
      this->ui.membraneBlendZTest,
      this->ui.particleBlendZTest,
   }) {
      constexpr const char* disambig = "blend z-test";
      using enumeration = loaded_form_type::z_test_function;

      widget->clear();
      widget->addItem(tr("Equal To", disambig), (int)enumeration::equal);
      widget->addItem(tr("Normal", disambig), (int)enumeration::normal);
      widget->addItem(tr("Greater Than", disambig), (int)enumeration::greater);
      widget->addItem(tr("Greater or Equal", disambig), (int)enumeration::greater_equal);
      widget->addItem(tr("Always Show", disambig), (int)enumeration::always);
   }

   ui::bind(this->ui.membraneFillTexture, this->ui.membraneFillTexturePreview);
   ui::bind(this->ui.membranePaletteTexture, this->ui.membranePaletteTexturePreview);
   ui::bind(this->ui.membraneHoleTexture, this->ui.membraneHoleTexturePreview);
   ui::bind(this->ui.particleTexture, this->ui.particleTexturePreview);
   ui::bind(this->ui.particlePaletteTexture, this->ui.particlePaletteTexturePreview);
   QObject::connect(this->ui.membraneFillFlagIgnoreAlpha, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.membraneFillTexturePreview->setShowAlpha(!checked);
   });
   QObject::connect(
      this->ui.membraneFillFlagGreyscaleToPaletteAlpha,
      &QCheckBox::toggled,
      this->ui.membranePaletteTexturePreview,
      &DKTextureAssetPane::setShowAlpha
   );
   QObject::connect(
      this->ui.particleFlagGreyscaleToPaletteAlpha,
      &QCheckBox::toggled,
      this->ui.particlePaletteTexturePreview,
      &DKTextureAssetPane::setShowAlpha
   );

   this->load(); // this creates the working copy.
}
void FormDialogEffectShader::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.soundAmbient, working.ambient_sound, working);
   ui::bind_inverse(this->ui.flagEnableMembrane, working.flags, loaded_form_type::flag::no_membrane);
   ui::bind_inverse(this->ui.flagEnableParticle, working.flags, loaded_form_type::flag::no_particle);
   ui::bind(this->ui.flagBloodGeom, working.flags, loaded_form_type::flag::use_blood_geom);

   #pragma region Membrane options
      ui::bind(this->ui.membraneBlendModeSrc, working.membrane.blend.src);
      ui::bind(this->ui.membraneBlendModeDst, working.membrane.blend.dst);
      ui::bind(this->ui.membraneBlendOp,      working.membrane.blend.op);
      ui::bind(this->ui.membraneBlendZTest,   working.membrane.blend.z_test);
      ui::bind(this->ui.flagMembraneIgnoreBaseAlpha, working.flags, loaded_form_type::flag::ignore_base_geom_alpha);
      ui::bind(this->ui.flagMembraneSkinOnly,        working.flags, loaded_form_type::flag::skin_only);
      #pragma region Fill/Texture
         ui::bind(this->ui.membraneFillAlphaTimeFadeIn,     working.membrane.fill.alpha.times.fade_in);
         ui::bind(this->ui.membraneFillAlphaTimeFull,       working.membrane.fill.alpha.times.full);
         ui::bind(this->ui.membraneFillAlphaTimeFadeOut,    working.membrane.fill.alpha.times.fade_out);
         ui::bind(this->ui.membraneFillAlphaRatioFull,      working.membrane.fill.alpha.ratios.full);
         ui::bind(this->ui.membraneFillAlphaRatioPersist,   working.membrane.fill.alpha.ratios.persistent);
         ui::bind(this->ui.membraneFillAlphaPulseAmplitude, working.membrane.fill.alpha.pulse.amplitude);
         ui::bind(this->ui.membraneFillAlphaPulseFrequency, working.membrane.fill.alpha.pulse.frequency);
         ui::bind(this->ui.membraneFillTextureSpeedU, working.membrane.fill.textures.speed.u);
         ui::bind(this->ui.membraneFillTextureSpeedV, working.membrane.fill.textures.speed.v);
         ui::bind(this->ui.membraneFillTextureScaleU, working.membrane.fill.textures.scale.u);
         ui::bind(this->ui.membraneFillTextureScaleV, working.membrane.fill.textures.scale.v);
         #pragma region Color keys
            ui::bind(this->ui.membraneColorKey1Color, working.membrane.fill.color_keys[0].color);
            ui::bind(this->ui.membraneColorKey2Color, working.membrane.fill.color_keys[1].color);
            ui::bind(this->ui.membraneColorKey3Color, working.membrane.fill.color_keys[2].color);
            ui::bind(this->ui.membraneColorKey1Scale, working.membrane.fill.color_keys[0].alpha);
            ui::bind(this->ui.membraneColorKey2Scale, working.membrane.fill.color_keys[1].alpha);
            ui::bind(this->ui.membraneColorKey3Scale, working.membrane.fill.color_keys[2].alpha);
            ui::bind(this->ui.membraneColorKey1Time,  working.membrane.fill.color_keys[0].time);
            ui::bind(this->ui.membraneColorKey2Time,  working.membrane.fill.color_keys[1].time);
            ui::bind(this->ui.membraneColorKey3Time,  working.membrane.fill.color_keys[2].time);
         #pragma endregion
         ui::bind(this->ui.membraneFillTexture,    working.membrane.fill.textures.main);
         ui::bind(this->ui.membranePaletteTexture, working.membrane.fill.textures.palette);
         ui::bind(this->ui.membraneFillFlagIgnoreAlpha,             working.flags, loaded_form_type::flag::ignore_alpha);
         ui::bind(this->ui.membraneFillFlagProjectedUVs,            working.flags, loaded_form_type::flag::projected_uv);
         ui::bind(this->ui.membraneFillFlagLighting,                working.flags, loaded_form_type::flag::lighting);
         ui::bind(this->ui.membraneFillFlagNoWeapons,               working.flags, loaded_form_type::flag::no_weapons);
         ui::bind(this->ui.membraneFillFlagGreyscaleToPaletteColor, working.flags, loaded_form_type::flag::membrane_greyscale_color);
         ui::bind(this->ui.membraneFillFlagGreyscaleToPaletteAlpha, working.flags, loaded_form_type::flag::membrane_greyscale_alpha);
      #pragma endregion
      #pragma region Edge
         ui::bind(this->ui.membraneEdgeAlphaTimeFadeIn,     working.membrane.edge.alpha.times.fade_in);
         ui::bind(this->ui.membraneEdgeAlphaTimeFull,       working.membrane.edge.alpha.times.full);
         ui::bind(this->ui.membraneEdgeAlphaTimeFadeOut,    working.membrane.edge.alpha.times.fade_out);
         ui::bind(this->ui.membraneEdgeAlphaRatioFull,      working.membrane.edge.alpha.ratios.full);
         ui::bind(this->ui.membraneEdgeAlphaRatioPersist,   working.membrane.edge.alpha.ratios.persistent);
         ui::bind(this->ui.membraneEdgeAlphaPulseAmplitude, working.membrane.edge.alpha.pulse.amplitude);
         ui::bind(this->ui.membraneEdgeAlphaPulseFrequency, working.membrane.edge.alpha.pulse.frequency);

         ui::bind(this->ui.membraneEdgeColor,       working.membrane.edge.color);
         ui::bind(this->ui.membraneEdgeFalloff,     working.membrane.edge.falloff);
         ui::bind(this->ui.membraneFlagEdgeInverse, working.flags, loaded_form_type::flag::invert_edge_effect);
      #pragma endregion
      #pragma region Holes
         ui::bind(this->ui.membraneHoleTexture,    working.membrane.fill.textures.holes);
         ui::bind(this->ui.membraneHoleAlphaStart, working.membrane.holes.start_value);
         ui::bind(this->ui.membraneHoleAlphaEnd,   working.membrane.holes.end_value);
         ui::bind(this->ui.membraneHoleTimeStart,  working.membrane.holes.start_time);
         ui::bind(this->ui.membraneHoleTimeEnd,    working.membrane.holes.end_time);
      #pragma endregion
   #pragma endregion
   #pragma region Particle options
      ui::bind(this->ui.particleBlendModeSrc, working.particle.blend.src);
      ui::bind(this->ui.particleBlendModeDst, working.particle.blend.dst);
      ui::bind(this->ui.particleBlendOp,      working.particle.blend.op);
      ui::bind(this->ui.particleBlendZTest,   working.particle.blend.z_test);
      #pragma region Graphics
         ui::bind(this->ui.particleTexture,        working.particle.textures.main);
         ui::bind(this->ui.particlePaletteTexture, working.particle.textures.palette);
         ui::bind(this->ui.particleTextureCountU,  working.particle.textures.count.u);
         ui::bind(this->ui.particleTextureCountV,  working.particle.textures.count.v);
         ui::bind(this->ui.particleFlagGreyscaleToPaletteColor, working.flags, loaded_form_type::flag::particle_greyscale_color);
         ui::bind(this->ui.particleFlagGreyscaleToPaletteAlpha, working.flags, loaded_form_type::flag::particle_greyscale_alpha);
         #pragma region Animated
            ui::bind(this->ui.particleFlagAnimated, working.flags, loaded_form_type::flag::particle_animated);
            ui::bind(this->ui.particleAnimStart,              working.particle.textures.animation.start_frame.base);
            ui::bind(this->ui.particleAnimStartVariance,      working.particle.textures.animation.start_frame.variance);
            ui::bind(this->ui.particleAnimLoopStart,          working.particle.textures.animation.loop_start_frame.base);
            ui::bind(this->ui.particleAnimLoopStartVariance,  working.particle.textures.animation.loop_start_frame.variance);
            ui::bind(this->ui.particleAnimEnd,                working.particle.textures.animation.end_frame);
            ui::bind(this->ui.particleAnimFrameCount,         working.particle.textures.animation.frame_count.base);
            ui::bind(this->ui.particleAnimFrameCountVariance, working.particle.textures.animation.frame_count.variance);
         #pragma endregion
         #pragma region Color keys
            ui::bind(this->ui.particleColorKey1Color, working.particle.color_keys[0].color);
            ui::bind(this->ui.particleColorKey2Color, working.particle.color_keys[1].color);
            ui::bind(this->ui.particleColorKey3Color, working.particle.color_keys[2].color);
            ui::bind(this->ui.particleColorKey1Scale, working.particle.color_keys[0].alpha);
            ui::bind(this->ui.particleColorKey2Scale, working.particle.color_keys[1].alpha);
            ui::bind(this->ui.particleColorKey3Scale, working.particle.color_keys[2].alpha);
            ui::bind(this->ui.particleColorKey1Time,  working.particle.color_keys[0].time);
            ui::bind(this->ui.particleColorKey2Time,  working.particle.color_keys[1].time);
            ui::bind(this->ui.particleColorKey3Time,  working.particle.color_keys[2].time);
         #pragma endregion
      #pragma endregion
      #pragma region Behavior
         ui::bind(this->ui.particleBirthTimeRampUp,   working.particle.behavior.spawn.times.ramp_up);
         ui::bind(this->ui.particleBirthTimeFull,     working.particle.behavior.spawn.times.full);
         ui::bind(this->ui.particleBirthTimeRampDown, working.particle.behavior.spawn.times.ramp_down);
         ui::bind(this->ui.particleCountFull,         working.particle.behavior.spawn.counts.full);
         ui::bind(this->ui.particleCountPersist,      working.particle.behavior.spawn.counts.persistent);
         ui::bind(this->ui.particleEmitDepthLimit,    working.particle.behavior.spawn.scene_graph_emit_depth_limit);
         ui::bind(this->ui.particleLifetime,          working.particle.behavior.lifetime.base);
         ui::bind(this->ui.particleLifetimeVariance,  working.particle.behavior.lifetime.variance);
         #pragma region Transforms over time
            ui::bind(this->ui.particleScaleKey1,          working.particle.scale_keys[0].scale);
            ui::bind(this->ui.particleScaleKey2,          working.particle.scale_keys[1].scale);
            ui::bind(this->ui.particleScaleKey1Time,      working.particle.scale_keys[0].time);
            ui::bind(this->ui.particleScaleKey2Time,      working.particle.scale_keys[1].time);
            ui::bind(this->ui.particleExplosionWindSpeed, working.particle.behavior.movement.explosion_wind_speed);
         #pragma endregion
         #pragma region Positioning
            ui::bind(this->ui.particleSpawnOffset,          working.particle.behavior.movement.initial_position.base);
            ui::bind(this->ui.particleSpawnOffsetVariance,  working.particle.behavior.movement.initial_position.variance);
            ui::bind(this->ui.particleInitialSpeed,         working.particle.behavior.movement.initial_speed.base);
            ui::bind(this->ui.particleInitialSpeedVariance, working.particle.behavior.movement.initial_speed.variance);
            ui::bind(this->ui.particleInitialVelocityX, working.particle.behavior.movement.initial_velocity.x);
            ui::bind(this->ui.particleInitialVelocityY, working.particle.behavior.movement.initial_velocity.y);
            ui::bind(this->ui.particleInitialVelocityZ, working.particle.behavior.movement.initial_velocity.z);
            ui::bind(this->ui.particleAccelX, working.particle.behavior.movement.acceleration.absolute.x);
            ui::bind(this->ui.particleAccelY, working.particle.behavior.movement.acceleration.absolute.y);
            ui::bind(this->ui.particleAccelZ, working.particle.behavior.movement.acceleration.absolute.z);
            ui::bind(this->ui.particleInitialRotation,         working.particle.behavior.movement.initial_rotation.base);
            ui::bind(this->ui.particleInitialRotationVariance, working.particle.behavior.movement.initial_rotation.variance);
            ui::bind(this->ui.particleRotationSpeed,           working.particle.behavior.movement.rotation_speed.base);
            ui::bind(this->ui.particleRotationSpeedVariance,   working.particle.behavior.movement.rotation_speed.variance);
         #pragma endregion
      #pragma endregion
      #pragma region Debris
         ui::bind(this->ui.particleDebris,     working.particle.debris.form, working);
         ui::bind(this->ui.debrisTimeFadeIn,   working.particle.debris.times.fade_in);
         ui::bind(this->ui.debrisTimeFadeOut,  working.particle.debris.times.fade_out);
         ui::bind(this->ui.debrisScaleMin,     working.particle.debris.scales.start);
         ui::bind(this->ui.debrisScaleMax,     working.particle.debris.scales.end);
         ui::bind(this->ui.debrisTimeScaleIn,  working.particle.debris.times.scale_in);
         ui::bind(this->ui.debrisTimeScaleOut, working.particle.debris.times.scale_out);
      #pragma endregion
   #pragma endregion

   this->ui.membraneFillTexturePreview->setShowAlpha(!this->ui.membraneFillFlagIgnoreAlpha->isChecked());
   this->ui.membranePaletteTexturePreview->setShowAlpha(this->ui.membraneFillFlagGreyscaleToPaletteAlpha->isChecked());
   this->ui.particlePaletteTexturePreview->setShowAlpha(this->ui.particleFlagGreyscaleToPaletteAlpha->isChecked());
}
void FormDialogEffectShader::_save_impl() {
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