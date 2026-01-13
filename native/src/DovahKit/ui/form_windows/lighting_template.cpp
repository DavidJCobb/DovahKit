#include "./lighting_template.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/InteriorCellPickerFilter.h"

FormDialogLightingTemplate::FormDialogLightingTemplate(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.copyFromCellForm->setAllowedFormType(dovah::form_type::cell);
   {
      auto* filter = new InteriorCellPickerFilter(this);
      this->ui.copyFromCellForm->setCustomFilter(filter);
   }

   ui::set_unsigned_range<float>(this->ui.directionalFade);
   ui::set_unsigned_range<float>(this->ui.fogDistNear);
   ui::set_unsigned_range<float>(this->ui.fogDistFar);
   ui::set_unsigned_range<float>(this->ui.fogDistClip);
   ui::set_unsigned_range<float>(this->ui.omniFadeStart);
   ui::set_unsigned_range<float>(this->ui.omniFadeEnd);
   ui::set_unsigned_range<float>(this->ui.fresnel);

   this->load(); // this creates the working copy.
}
void FormDialogLightingTemplate::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.ambientColor,     working.data.ambient.base);
   ui::bind(this->ui.directionalColor, working.data.directional.color);
   ui::bind(this->ui.directionalFade,  working.data.directional.fade);
   ui::bind(this->ui.directionalRotXY, working.data.directional.rotation.xy);
   ui::bind(this->ui.directionalRotZ,  working.data.directional.rotation.z);
   ui::bind(this->ui.dalcXNeg, working.directional_ambient.x.negative);
   ui::bind(this->ui.dalcXPos, working.directional_ambient.x.positive);
   ui::bind(this->ui.dalcYNeg, working.directional_ambient.y.negative);
   ui::bind(this->ui.dalcYPos, working.directional_ambient.y.positive);
   ui::bind(this->ui.dalcZNeg, working.directional_ambient.z.negative);
   ui::bind(this->ui.dalcZPos, working.directional_ambient.z.positive);
   ui::bind(this->ui.fogColorNear, working.data.fog.colors.near);
   ui::bind(this->ui.fogColorFar,  working.data.fog.colors.far);
   ui::bind(this->ui.fogDistNear,  working.data.fog.near);
   ui::bind(this->ui.fogDistFar,   working.data.fog.far);
   ui::bind(this->ui.fogDistClip,  working.data.fog.clip_distance);
   ui::bind(this->ui.fogMax,       working.data.fog.max);
   ui::bind(this->ui.fogPow,       working.data.fog.power);
   ui::bind(this->ui.omniFadeStart, working.data.light_fade_distance.start);
   ui::bind(this->ui.omniFadeEnd,   working.data.light_fade_distance.end);
   ui::bind(this->ui.specular, working.directional_ambient.specular);
   ui::bind(this->ui.fresnel,  working.directional_ambient.fresnel);

   QObject::connect(this->ui.buttonSetDalcFromAmb, &QPushButton::clicked, this, [this]() {
      auto& working = *this->form;
      auto& dalc    = working.directional_ambient;
      dalc.set_from_ambient(working.data.ambient.base);
      _pull_dalc_to_ui();
   });

   QObject::connect(this->ui.buttonCopyFromCell, &QPushButton::clicked, this, &FormDialogLightingTemplate::_copy_from_cell);
}
void FormDialogLightingTemplate::_save_impl() {
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

#include "dovah/forms/Cell.h"
void FormDialogLightingTemplate::_copy_from_cell() {
   auto* cell = this->ui.copyFromCellForm->formStub();
   if (!cell || cell->is_exterior_cell())
      return;
   auto loaded_cell = cell->load().ptr_cast<dovah::loaded_forms::Cell>();
   if (!loaded_cell)
      return;

   auto& working = *this->form;

   auto& cell_lighting_data = loaded_cell->interior.lighting;
   auto* cell_template_stub = loaded_cell->interior.lighting_template.get_form_stub();
   dovah::loaded_form_ptr<dovah::loaded_forms::LightingTemplate> cell_template_form;
   if (cell_template_stub && cell_template_stub->form_type == dovah::form_type::lighting_template)
      cell_template_form = cell_template_stub->load().ptr_cast<dovah::loaded_forms::LightingTemplate>();

   using inherit_flag = dovah::loaded_forms::structs::cell_lighting::inherit_flag;
   auto cell_inherit_flags = cell_lighting_data.inherit_flags;
   if (!cell_template_form) {
      cell_inherit_flags = 0;
   }

   if (cell_inherit_flags & inherit_flag::ambient) {
      working.data.ambient.base        = cell_template_form->data.ambient.base;
      working.data.ambient.directional = cell_template_form->directional_ambient;
   } else {
      working.data.ambient = cell_lighting_data.ambient;
   }
   _pull_dalc_to_ui();

   #define X(flag, field) \
      if (cell_inherit_flags & inherit_flag::flag) { \
         working.data.field = cell_template_form->data.field; \
      } else { \
         working.data.field = cell_lighting_data.field; \
      }

   X(directional,          directional.color);
   X(fog_color,            fog.colors.near);
   X(fog_color,            fog.colors.far);
   X(fog_distance_near,    fog.near);
   X(fog_distance_far,     fog.far);
   X(directional_rotation, directional.rotation.xy);
   X(directional_rotation, directional.rotation.z);
   X(directional_fade,     directional.fade);
   X(fog_clip_distance,    fog.clip_distance);
   X(fog_power,            fog.power);
   X(fog_max,              fog.max);
   X(light_fade_distances, light_fade_distance.start);
   X(light_fade_distances, light_fade_distance.end);

   #undef X

   const auto blockers = std::array{
      QSignalBlocker(this->ui.directionalColor),
      QSignalBlocker(this->ui.directionalFade),
      QSignalBlocker(this->ui.directionalRotXY),
      QSignalBlocker(this->ui.directionalRotZ),
      QSignalBlocker(this->ui.fogColorFar),
      QSignalBlocker(this->ui.fogColorNear),
      QSignalBlocker(this->ui.fogDistClip),
      QSignalBlocker(this->ui.fogDistFar),
      QSignalBlocker(this->ui.fogDistNear),
      QSignalBlocker(this->ui.fogMax),
      QSignalBlocker(this->ui.fogPow),
      QSignalBlocker(this->ui.omniFadeEnd),
      QSignalBlocker(this->ui.omniFadeStart),
   };
   {
      auto& color = working.data.directional.color;
      this->ui.directionalColor->setColor(QColor(color.r, color.g, color.b));
   }
   this->ui.directionalFade->setValue(working.data.directional.fade);
   this->ui.directionalRotXY->setValue(working.data.directional.rotation.xy);
   this->ui.directionalRotZ->setValue(working.data.directional.rotation.z);
   {
      auto& color = working.data.fog.colors.far;
      this->ui.fogColorFar->setColor(QColor(color.r, color.g, color.b));
   }
   {
      auto& color = working.data.fog.colors.near;
      this->ui.fogColorNear->setColor(QColor(color.r, color.g, color.b));
   }
   this->ui.fogDistClip->setValue(working.data.fog.clip_distance);
   this->ui.fogDistFar->setValue(working.data.fog.far);
   this->ui.fogDistNear->setValue(working.data.fog.near);
   this->ui.fogMax->setValue(working.data.fog.max);
   this->ui.fogPow->setValue(working.data.fog.power);
   this->ui.omniFadeEnd->setValue(working.data.light_fade_distance.end);
   this->ui.omniFadeStart->setValue(working.data.light_fade_distance.start);
}
void FormDialogLightingTemplate::_pull_dalc_to_ui() {
   const auto blockers = std::array{
      this->ui.dalcXNeg,
      this->ui.dalcXPos,
      this->ui.dalcYNeg,
      this->ui.dalcYPos,
      this->ui.dalcZNeg,
      this->ui.dalcZPos,
   };
   auto& working = *this->form;
   auto& dalc    = working.directional_ambient;
   this->ui.dalcXNeg->setColor(QColor::fromRgb(
      dalc.x.negative.r,
      dalc.x.negative.g,
      dalc.x.negative.b
   ));
   this->ui.dalcXPos->setColor(QColor::fromRgb(
      dalc.x.positive.r,
      dalc.x.positive.g,
      dalc.x.positive.b
   ));
   this->ui.dalcYNeg->setColor(QColor::fromRgb(
      dalc.y.negative.r,
      dalc.y.negative.g,
      dalc.y.negative.b
   ));
   this->ui.dalcYPos->setColor(QColor::fromRgb(
      dalc.y.positive.r,
      dalc.y.positive.g,
      dalc.y.positive.b
   ));
   this->ui.dalcZNeg->setColor(QColor::fromRgb(
      dalc.z.negative.r,
      dalc.z.negative.g,
      dalc.z.negative.b
   ));
   this->ui.dalcZPos->setColor(QColor::fromRgb(
      dalc.z.positive.r,
      dalc.z.positive.g,
      dalc.z.positive.b
   ));
}