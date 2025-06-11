#include "./material_type.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

FormDialogMaterialType::FormDialogMaterialType(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);

   this->ui.name->setMaxLength(loaded_form_type::max_name_length);
   this->ui.havokImpactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
   this->ui.parent->setAllowedFormType(dovah::form_type::material_type);
   this->ui.parent->setCustomFilter(this->_filters.exclude_self);

   this->load(); // this creates the working copy.
}
void FormDialogMaterialType::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(this->formStub());

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.name, working.name);
   ui::bind(this->ui.parent, working.parent, working);
   ui::bind(this->ui.buoyancy, working.buoyancy);
   ui::bind(this->ui.havokImpactDataSet, working.impact_data_set, working);
   {
      auto* widget = this->ui.havokColor;
      auto& values = working.color;
      widget->setColor(QColor(values.r / 255.0F, values.g / 255.0F, values.b / 255.0F));
      QObject::connect(widget, &DKColorPickerButton::colorChanged, this, [this, &values](QColor c) {
         values.r = c.redF();
         values.g = c.greenF();
         values.b = c.blueF();
      });
   }

   ui::bind(this->ui.flagArrowsStick, working.flags, loaded_form_type::flag::arrows_stick);
   ui::bind(this->ui.flagStairs,      working.flags, loaded_form_type::flag::stairs);
}
void FormDialogMaterialType::_save_impl() {
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