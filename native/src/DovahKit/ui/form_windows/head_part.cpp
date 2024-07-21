#include "./head_part.h"
#include <limits>
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogHeadPart::FormDialogHeadPart(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* widget = this->ui.type;
      for (size_t i = 0; i < widget->count(); ++i)
         widget->setItemData(i, i, Qt::ItemDataRole::UserRole);
   }

   this->ui.color->setAllowedFormType(dovah::form_type::color);
   this->ui.textureSet->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.validRaces->setAllowedFormType(dovah::form_type::formlist);

   this->ui.extraParts->setAllowedFormTypes({ dovah::form_type::head_part });
   QObject::connect(this->ui.flagIsExtraPart, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.extraParts->setEnabled(!checked);
   });

   this->load(); // this creates the working copy.
}
void FormDialogHeadPart::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(editor.convert_localized_string(working.name));
   {  // Sex
      auto* widget = this->ui.sex;
      if (working.flags & loaded_form_type::head_part_flag::female) {
         if (working.flags & loaded_form_type::head_part_flag::male) {
            widget->setCurrentIndex(0);
         } else {
            widget->setCurrentIndex(1);
         }
      } else if (working.flags & loaded_form_type::head_part_flag::male) {
         widget->setCurrentIndex(2);
      } else {
         widget->setCurrentIndex(0);
      }
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         if (index < 0)
            return;
         auto& dst = this->form->flags;
         switch (index) {
            case 0:
               dst &= ~loaded_form_type::head_part_flag::female;
               dst &= ~loaded_form_type::head_part_flag::male;
               break;
            case 1:
               dst |=  loaded_form_type::head_part_flag::female;
               dst &= ~loaded_form_type::head_part_flag::male;
               break;
            case 2:
               dst &= ~loaded_form_type::head_part_flag::female;
               dst |=  loaded_form_type::head_part_flag::male;
               break;
         }
      });
   }
   ui::bind(this->ui.flagPlayable,     working.flags, loaded_form_type::head_part_flag::playable); // There's a "non-playable" record flag, but Bethesda doesn't seem to use it.
   ui::bind(this->ui.flagUseSolidTint, working.flags, loaded_form_type::head_part_flag::use_solid_tint);
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.tri,          working.morphs.tri);
   ui::bind(this->ui.raceMorph,    working.morphs.race);
   ui::bind(this->ui.charGenMorph, working.morphs.chargen);
   ui::bind(this->ui.color,        working.color, working);
   ui::bind(this->ui.textureSet,   working.texture_set, working);
   ui::bind(this->ui.validRaces,   working.valid_races, working);
   ui::bind(this->ui.flagIsExtraPart, working.flags, loaded_form_type::head_part_flag::is_extra_part);

   for (auto& ref : working.extra_parts) {
      this->ui.extraParts->addStub(ref.get_form_stub());
   }
}
void FormDialogHeadPart::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);

   this->ui.extraParts->commitStubs(working.extra_parts, working);
}