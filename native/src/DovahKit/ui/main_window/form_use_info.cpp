#include "form_use_info.h"
#include "../../editor/core.h"
#include "../../helpers/qt/strings.h"

FormUseInfoDialog::FormUseInfoDialog(const dovah::form_stub* stub, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   this->stub = stub;
   //
   this->ui.usesInGeneral->setRelationshipMode(FormUseInfoList::relationship_mode::general_only);
   this->ui.usesAsBaseForm->setRelationshipMode(FormUseInfoList::relationship_mode::base_form_only);
   this->ui.usesInGeneral->setTextFilter(this->ui.filterText);
   this->ui.usesAsBaseForm->setTextFilter(this->ui.filterText);
   //
   this->ui.filterSignature->setAllowUnfiltered(true);
   QObject::connect(this->ui.filterSignature, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
      dovah::form_type_t data = this->ui.filterSignature->formType();
      this->ui.usesInGeneral->setFormTypeFilter(data);
      this->ui.usesAsBaseForm->setFormTypeFilter(data);
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->stub = nullptr;
      this->reject();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      if (stub == this->stub) {
         this->stub = nullptr;
         this->reject();
      }
   });
   //
   this->rebuild();
}
void FormUseInfoDialog::rebuild() {
   if (this->stub) {
      uint32_t signature = dovah::form_type_info::lookup(this->stub->formType).signature;
      QString  formID    = QString("%1").arg(this->stub->formID, 8, 16, QChar('0')).toUpper();
      this->setWindowTitle(tr("Use Info Report for [%1:%2]%3", "use info report").arg(cobb::qt::four_cc_to_string(signature)).arg(formID).arg(this->stub->get_editor_id()));
   } else {
      this->setWindowTitle(tr("Use Info Report", "use info report"));
   }
   //
   this->ui.usesInGeneral->setTarget(this->stub);
   this->ui.usesAsBaseForm->setTarget(this->stub);
   this->ui.usesInGeneral->build();
   this->ui.usesAsBaseForm->build();
   //
   if (this->stub) {
      bool is_reference = dovah::form_type_info::form_type_is_reference(this->stub->formType);
      this->ui.labelUsesAsBaseForm->setVisible(!is_reference);
      this->ui.usesAsBaseForm->setVisible(!is_reference);
   }
}