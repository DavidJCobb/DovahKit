#include "./grass.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogGrass::FormDialogGrass(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::set_unsigned_range<float>(this->ui.rangePos);
   ui::set_unsigned_range<float>(this->ui.rangeHeight);
   ui::set_unsigned_range<float>(this->ui.rangeColor);
   ui::set_unsigned_range<float>(this->ui.wavePeriod);

   {
      auto* widget = this->ui.shoreComparator;
      widget->clear();
      widget->addItem(tr("Above - At Least"),  (int)loaded_form_type::units_from_water_comparator::above_at_least);
      widget->addItem(tr("Above - At Most"),   (int)loaded_form_type::units_from_water_comparator::above_at_most);
      widget->addItem(tr("Below - At Least"),  (int)loaded_form_type::units_from_water_comparator::below_at_least);
      widget->addItem(tr("Below - At Most"),   (int)loaded_form_type::units_from_water_comparator::below_at_most);
      widget->addItem(tr("Either - At Least"), (int)loaded_form_type::units_from_water_comparator::either_at_least);
      widget->addItem(tr("Either - At Most"),  (int)loaded_form_type::units_from_water_comparator::either_at_most);
      widget->addItem(tr("Either - At Most Above"), (int)loaded_form_type::units_from_water_comparator::either_at_most_above);
      widget->addItem(tr("Either - At Most Below"), (int)loaded_form_type::units_from_water_comparator::either_at_most_below);
   }
   ui::set_range<uint16_t>(this->ui.shoreUnits);

   this->load(); // this creates the working copy.
}
void FormDialogGrass::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.model->initializeFrom(working.model);

   ui::bind(this->ui.density, working.density);
   ui::bind(this->ui.slopeMin, working.slope.min);
   ui::bind(this->ui.slopeMax, working.slope.max);
   ui::bind(this->ui.rangePos, working.position_range);
   ui::bind(this->ui.rangeHeight, working.height_range);
   ui::bind(this->ui.rangeColor, working.color_range);
   ui::bind(this->ui.wavePeriod, working.wave_period);

   ui::bind(this->ui.flagVertexLighting, working.grass_flags, loaded_form_type::grass_flag::vertex_lighting);
   ui::bind(this->ui.flagUniformScaling, working.grass_flags, loaded_form_type::grass_flag::uniform_scaling);
   ui::bind(this->ui.flagFitToSlope,     working.grass_flags, loaded_form_type::grass_flag::fit_to_slope);

   ui::bind(this->ui.shoreComparator, working.distance_from_water.comparator);
   ui::bind(this->ui.shoreUnits, working.distance_from_water.units);
}
void FormDialogGrass::_save_impl() {
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