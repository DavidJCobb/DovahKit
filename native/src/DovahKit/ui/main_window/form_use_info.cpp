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
   {
      auto widget = this->ui.filterSignature;
      widget->addItem(tr(" ANY ", "use info filter by signature"), int(dovah::form_type::none));
      for (auto& info : dovah::form_types) {
         switch (info.formType) {
            case dovah::form_type::none:
            case dovah::form_type::file_header:
            case dovah::form_type::file_record_group:
            case dovah::form_type::setting:
               continue;
         }
         auto signature = QString("%1%2%3%4")
            .arg(QChar(info.signature >> 0x18))
            .arg(QChar((info.signature >> 0x10) & 0xFF))
            .arg(QChar((info.signature >> 0x08) & 0xFF))
            .arg(QChar(info.signature & 0xFF));
         widget->addItem(signature, int(info.formType));
      }
      auto model = widget->model();
      if (model)
         model->sort(0, Qt::AscendingOrder);
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         dovah::form_type_t data = this->ui.filterSignature->itemData(index, Qt::UserRole).toInt();
         this->ui.usesInGeneral->setFormTypeFilter(data);
         this->ui.usesAsBaseForm->setFormTypeFilter(data);
      });
   }
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->stub = nullptr;
      //this->rebuild(); // FormUseInfoList does this on its own
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
}