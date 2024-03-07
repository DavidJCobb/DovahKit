#include "./DKAddPapyrusScriptDialog.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include "ui/models/papyrus/DKAddPapyrusScriptModel.h"

DKAddPapyrusScriptDialog::DKAddPapyrusScriptDialog(QWidget* parent) : QDialog(parent) {
   QWidget* last_tabbable_widget = nullptr;

   auto* layout = new QGridLayout(this);
   int row = 0;
   {
      auto* wrapper = new QWidget(this);
      layout->addWidget(wrapper, row, 0);

      auto* sublayout = new QHBoxLayout(wrapper);
      wrapper->setLayout(sublayout);
      sublayout->setContentsMargins(0, 0, 0, 0);

      auto* label  = new QLabel(tr("Filter:"), this);
      auto* widget = this->_subwidgets.search = new QLineEdit(this);
      label->setBuddy(widget);
      sublayout->addWidget(label);
      sublayout->addWidget(widget);

      auto* hidden = this->_subwidgets.hidden = new QCheckBox(tr("Show Hidden Scripts"), this);
      sublayout->addWidget(hidden);

      setTabOrder(widget, hidden);
      last_tabbable_widget = hidden;
   }
   ++row;
   {
      auto* widget = this->_subwidgets.listview = new QListView(this);
      layout->addWidget(widget, row, 0);

      auto* model = this->_model = new DKAddPapyrusScriptModel;
      widget->setModel(model);

      setTabOrder(last_tabbable_widget, widget);
      last_tabbable_widget = widget;
   }
   ++row;
   {
      auto* wrapper = new QWidget(this);
      layout->addWidget(wrapper, row, 0);

      auto* sublayout = new QHBoxLayout(wrapper);
      wrapper->setLayout(sublayout);
      sublayout->setContentsMargins(0, 0, 0, 0);

      auto* button_ok     = new QPushButton(tr("Add Script"), this);
      auto* button_cancel = new QPushButton(tr("Cancel"), this);

      sublayout->addStretch();
      sublayout->addWidget(button_ok);
      sublayout->addWidget(button_cancel);

      setTabOrder(last_tabbable_widget, button_ok);
      setTabOrder(button_ok, button_cancel);

      QObject::connect(button_ok, &QPushButton::clicked, this, [this]() {
         if (auto* sel = this->_subwidgets.listview->selectionModel()) {
            auto rows = sel->selectedRows();
            if (!rows.isEmpty()) {
               auto qmi  = rows[0];
               auto name = this->_model->data(qmi, Qt::DisplayRole).toString();
               this->_result = name.toStdString();
            }
         }
         this->accept();
      });
      QObject::connect(button_cancel, &QPushButton::clicked, this, [this]() {
         this->reject();
      });
   }

   // set initial size
   this->resize(450, 300);
   {
      auto sp = this->sizePolicy();
      sp.setHorizontalPolicy(QSizePolicy::Preferred);
      sp.setVerticalPolicy(QSizePolicy::Preferred);
      this->setSizePolicy(sp);
   }
   
   QObject::connect(this->_subwidgets.search, &QLineEdit::textEdited, this, [this](const QString& text) {
      this->_model->setSearchText(text);
   });
   QObject::connect(this->_subwidgets.hidden, &QCheckBox::toggled, this, [this](bool checked) {
      this->_model->setShowHiddenScripts(checked);
   });
}

dovah::form_type DKAddPapyrusScriptDialog::targetType() const noexcept {
   return this->_model->targetType();
}
void DKAddPapyrusScriptDialog::setTargetType(dovah::form_type ft) {
   this->_model->setTargetType(ft);
}

void DKAddPapyrusScriptDialog::setAlreadyAttachedScripts(const std::vector<std::string>& scripts) {
   this->_model->setAlreadyAttachedScripts(scripts);
}