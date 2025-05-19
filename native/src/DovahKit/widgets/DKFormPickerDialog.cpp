#include "./DKFormPickerDialog.h"
#include <QBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QHeaderView>
#include <QPushButton>
#include "./widget-models/DKFormPicker/DKFormPickerDialogModel.h"
#include "./DKHeaderView.h"
#include "editor/form_stub_meta_type.h"

DKFormPickerDialog::DKFormPickerDialog(QWidget* parent) : QDialog(parent) {
   auto* layout = new QVBoxLayout(this);

   this->_subwidgets.filter = new QLineEdit(this);
   this->_subwidgets.table  = new QTableView(this);
   auto* buttonOK = new QPushButton(tr("OK"), this);

   this->setLayout(layout);
   {
      auto* sublayout = new QHBoxLayout();
      layout->addItem(sublayout);

      auto* label = new QLabel(tr("Filter:"), this);
      label->setBuddy(this->_subwidgets.filter);

      sublayout->addWidget(label);
      sublayout->addWidget(this->_subwidgets.filter);
   }
   layout->addWidget(this->_subwidgets.table);
   layout->addWidget(buttonOK);
   
   this->_model = new ui::impl::DKFormPicker::DialogModel(this);
   if (!this->isVisible()) {
      this->_model->setUpdatesEnabled(false);
   }
   this->_subwidgets.table->setModel(this->_model);
   this->_subwidgets.table->setCornerButtonEnabled(false);
   this->_subwidgets.table->setWordWrap(false);
   this->_subwidgets.table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   this->_subwidgets.table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
   this->_subwidgets.table->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
   {
      auto* header = new DKHeaderView(Qt::Horizontal, this->_subwidgets.table);
      header->setFlexResizeEnabled(true);
      this->_subwidgets.table->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(this->_subwidgets.table->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      this->_updateColumnVisibility();
      {
         auto metrics = QFontMetrics(this->_subwidgets.table->font());
         header->setMinimumSectionSize(2);
         header->setColumnFlex(0, 1, 0);
         header->setColumnFlex(1, 0, 0, metrics.boundingRect("XMMX").width() * 1.5F + 4);
         header->setColumnFlex(2, 0, 0, metrics.boundingRect("00000000").width() * 1.5F + 4); // sets minimum size
      }
      header->modSectionSizeTo(2, 4); // mimics a user resize and shrinks the column
      header->setStretchLastSection(false);
   }
   if (auto* h = this->_subwidgets.table->verticalHeader()) {
      h->setVisible(false);
      h->setSectionResizeMode(QHeaderView::ResizeToContents);
   }
   //
   QObject::connect(this->_subwidgets.table->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& qmi, const QModelIndex& prev_qmi) {
      auto data = this->_model->data(qmi, ui::impl::DKFormPicker::DialogModel::FormStubRole);
      this->_value = data.value<dovah::form_stub*>();
   });

   QObject::connect(this->_subwidgets.filter, &QLineEdit::textEdited, this, [this](const QString& text) {
      this->_model->setFilterString(text);
   });

   QObject::connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
}

void DKFormPickerDialog::setAllowedFormTypes(QList<dovah::form_type> ft) noexcept {
   if (this->_properties.allowed_form_types == ft)
      return;
   this->_properties.allowed_form_types = ft;
   this->_model->setAllowedFormTypes(this->_properties.allowed_form_types);
   this->_updateColumnVisibility();
}

void DKFormPickerDialog::addAllowedFormType(dovah::form_type ft) {
   if (this->_properties.allowed_form_types.contains(ft))
      return;
   this->_properties.allowed_form_types.append(ft);
   this->_model->setAllowedFormTypes(this->_properties.allowed_form_types);
   this->_updateColumnVisibility();
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

#if !defined(QT_PLUGIN)
   DKCustomFormFilter* DKFormPickerDialog::customFilter() const {
      return this->_model->get_custom_filter();
   }
   void DKFormPickerDialog::setCustomFilter(DKCustomFormFilter* v) {
      this->_model->set_custom_filter(v);
   }
#endif

void DKFormPickerDialog::_updateColumnVisibility() {
   if (!this->isVisible())
      return;
   auto* header = this->_subwidgets.table->horizontalHeader();
   if (!header)
      return;
   header->setSectionHidden(1, this->allowedFormTypes().size() == 1);

   for (size_t i = 0; i < 3; ++i)
      header->setSectionResizeMode(i, QHeaderView::Interactive);
   
   if (auto* casted = dynamic_cast<DKHeaderView*>(this->_subwidgets.table->horizontalHeader())) {
      //
      // Edge-case: we want to default the initial width of the form ID column to 4px (i.e. collapsed). 
      // However, if we do that before the type column is hidden, then when the type column is hidden, 
      // the form ID column may expand to fill the remaining space.
      // 
      // This is because we're not specifically saying, "Make the form ID column become 4px wide." 
      // Rather, we're saying, "Compute a modifier that will reduce the form ID column's width to 4px." 
      // When more space becomes available, the form ID column fills it, minus that now-outdated modifier.
      // 
      // Forcing the column width every time we update column visibility DOES mean that if the list of 
      // allowed form types changes while the user is able to interact with the list view (i.e. after 
      // it's displayed), then user changes to the form ID column size will be clobbered.
      // 
      casted->modSectionSizeTo(2, 4);
   }
}

/*virtual*/ void DKFormPickerDialog::showEvent(QShowEvent* event) /*override*/ {
   #if _DEBUG
      if (this->_properties.allowed_form_types.empty()) {
         qDebug(
            "WARNING: DKFormPickerDialog being shown with no form type restrictions. For Skyrim.esm it'll end up iterating over 200,000 forms!\n"
            "         If you're using a custom filter, that still has to run on 200K forms. Set a form type restriction too!!!"
         );
      }
   #endif
   this->_updateColumnVisibility();
   this->_model->setUpdatesEnabled(true);
}