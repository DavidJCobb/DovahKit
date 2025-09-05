#include "./shader_particle_geometry_data.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

FormDialogShaderParticleGeometryData::FormDialogShaderParticleGeometryData(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::bind(this->ui.texture, this->ui.texturePreview);
   ui::item_indices_to_data(this->ui.shaderType);

   ui::set_unsigned_range<int32_t>(this->ui.boxSize);
   this->ui.particleSizeX->setRange(0, 9999);
   this->ui.particleSizeY->setRange(0, 9999);
   this->ui.centerOffsetMin->setRange(-9999, 9999);
   this->ui.centerOffsetMax->setRange(-9999, 9999);
   this->ui.subtextureCountX->setRange(1, 9999);
   this->ui.subtextureCountY->setRange(1, 9999);
   this->ui.particleDensity->setRange(0, 9999);
   this->ui.velocityGravity->setRange(0, 9999);
   this->ui.velocityAngular->setRange(0, 360 * 20);
   this->ui.initialRotationRange->setRange(0, 360 * 20);

   this->load(); // this creates the working copy.
}
void FormDialogShaderParticleGeometryData::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.shaderType, working.type);
   ui::bind(this->ui.boxSize, working.box_size);
   ui::bind(this->ui.particleSizeX, working.particles.size.x);
   ui::bind(this->ui.particleSizeY, working.particles.size.y);
   ui::bind(this->ui.centerOffsetMin, working.center_offset.min);
   ui::bind(this->ui.centerOffsetMax, working.center_offset.max);
   ui::bind(this->ui.subtextureCountX, working.subtexture_count.x);
   ui::bind(this->ui.subtextureCountY, working.subtexture_count.y);
   ui::bind(this->ui.particleDensity, working.particles.density);
   ui::bind(this->ui.velocityGravity, working.gravity_velocity);
   ui::bind(this->ui.velocityAngular, working.rotation.velocity);
   ui::bind(this->ui.initialRotationRange, working.rotation.initial_range);
   ui::bind(this->ui.texture, working.particles.texture);
}
void FormDialogShaderParticleGeometryData::_save_impl() {
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