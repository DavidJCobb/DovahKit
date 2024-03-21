#include "./DKFormPickerDialog.h"
#include <QBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QHeaderView>
#include <QPushButton>
#include "./widget-models/DKFormPicker/DKFormPickerDialogModel.h"
#include "editor/form_stub_meta_type.h"

DKFormPickerDialog::DKFormPickerDialog(QWidget* parent) : QDialog(parent) {
   auto* layout = new QVBoxLayout(this);

   this->_subwidgets.filter = new QLineEdit(this);
   this->_subwidgets.table  = new QTableView(this);
   auto* buttonOK = new QPushButton(tr("OK"), this);

   this->setLayout(layout);
   {
      auto* sublayout = new QHBoxLayout(this);

      auto* label = new QLabel(tr("Filter:"), this);
      label->setBuddy(this->_subwidgets.filter);

      sublayout->addWidget(label);
      sublayout->addWidget(this->_subwidgets.filter);
      layout->addItem(sublayout);
   }
   layout->addWidget(this->_subwidgets.table);
   layout->addWidget(buttonOK);
   
   this->_model = new ui::impl::DKFormPicker::DialogModel(this);
   this->_model->setUpdatesEnabled(false);
   this->_subwidgets.table->setCornerButtonEnabled(false);
   this->_subwidgets.table->setWordWrap(false);
   if (auto* h = this->_subwidgets.table->horizontalHeader()) {
      h->setStretchLastSection(true);
   }
   if (auto* h = this->_subwidgets.table->verticalHeader()) {
      h->setVisible(false);
   }
   this->_subwidgets.table->setModel(this->_model);
   //
   QObject::connect(this->_subwidgets.table->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& qmi, const QModelIndex& prev_qmi) {
      auto data = this->_model->data(qmi, ui::impl::DKFormPicker::DialogModel::FormStubRole);
      this->_value = data.value<dovah::form_stub*>();
   });

   QObject::connect(this->_subwidgets.filter, &QLineEdit::textEdited, this, [this](const QString& text) {
      this->_model->setFilterString(text);
   });

   QObject::connect(buttonOK, &QPushButton::clicked, this, [this]() {
      this->accept();
   });
}

void DKFormPickerDialog::setAllowedFormTypes(QList<dovah::form_type> ft) noexcept {
   if (this->_properties.allowed_form_types == ft)
      return;
   this->_properties.allowed_form_types = ft;
   this->_model->setAllowedFormTypes(this->_properties.allowed_form_types);
}

void DKFormPickerDialog::addAllowedFormType(dovah::form_type ft) {
   if (this->_properties.allowed_form_types.contains(ft))
      return;
   this->_properties.allowed_form_types.append(ft);
   this->_model->setAllowedFormTypes(this->_properties.allowed_form_types);
}

void DKFormPickerDialog::setFormStub(dovah::form_stub* stub) noexcept {
   auto* table = this->_subwidgets.table;
   auto  qmi   = this->_model->index(stub);
   if (!qmi.isValid())
      return;

   const auto blocker = QSignalBlocker(table);
   table->setCurrentIndex(qmi);

   if (auto* sm = table->selectionModel()) {
      auto bottom_right  = qmi.siblingAtColumn(2);
      auto row_selection = QItemSelection(qmi, bottom_right);
      sm->select(row_selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);
   }

   this->_value = stub;
}

/*virtual*/ void DKFormPickerDialog::changeEvent(QEvent* event) /*override*/ {
   if (event->type() != QEvent::ParentChange)
      return;
   if (this->parentWidget()) {
      this->_model->setUpdatesEnabled(true);
   } else {
      this->_model->setUpdatesEnabled(false);
   }
}