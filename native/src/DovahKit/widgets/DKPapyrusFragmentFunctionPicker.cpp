#include "./DKPapyrusFragmentFunctionPicker.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#if !defined(QT_PLUGIN)
   #include "./widget-models/DKPapyrusFragmentFunctionModel.h"
#else
   #include "./DKPapyrusBoundScriptListPane.h"
#endif

DKPapyrusFragmentFunctionPicker::DKPapyrusFragmentFunctionPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   {
      auto* label = this->_subwidgets.header = new QLabel(tr("Script fragment data"), this);
      auto  font  = label->font();
      font.setBold(true);
      label->setFont(font);

      layout->addWidget(label, 0, 0, 1, 2);
   }
   {
      auto* label  = new QLabel(tr("Script:"), this);
      auto* widget = this->_subwidgets.scriptname = new QComboBox(this);
      label->setBuddy(widget);

      layout->addWidget(label,  1, 0);
      layout->addWidget(widget, 1, 1);
   }
   {
      auto* label  = new QLabel(tr("Function:"), this);
      auto* widget = this->_subwidgets.function = new QComboBox(this);
      label->setBuddy(widget);

      layout->addWidget(label,  2, 0);
      layout->addWidget(widget, 2, 1);
   }
   {
      auto* v_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      layout->addItem(v_spacer, 3, 0, 1, 2);
   }
   layout->setColumnStretch(0, 0);
   layout->setColumnStretch(1, 1);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.scriptname);
   this->setTabOrder(this->_subwidgets.scriptname, this->_subwidgets.function);

   #if !defined(QT_PLUGIN)
      this->_model = new DKPapyrusFragmentFunctionModel(this);
      this->_subwidgets.scriptname->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
      this->_subwidgets.scriptname->setEditable(true);
      this->_subwidgets.scriptname->lineEdit()->setMaxLength(std::numeric_limits<uint16_t>::max());
      this->_subwidgets.function->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
      this->_subwidgets.function->setEditable(true);
      this->_subwidgets.function->lineEdit()->setMaxLength(std::numeric_limits<uint16_t>::max());

      this->_subwidgets.scriptname->setModel(this->_model);
      this->_subwidgets.function->setModel(this->_model);
      this->_subwidgets.function->setRootModelIndex(this->_model->noneScriptQMI());

      QObject::connect(this->_subwidgets.scriptname, &QComboBox::editTextChanged, this, [this](const QString& name) {
         const auto none_qmi = this->_model->noneScriptQMI();
         if (name.isEmpty()) {
            auto i   = this->_subwidgets.scriptname->currentIndex();
            auto qmi = this->_model->index(i, 0, {});
            if (qmi.isValid()) {
               this->_subwidgets.function->setRootModelIndex(qmi);
               return;
            }
            this->_subwidgets.function->setRootModelIndex(none_qmi);
            this->_subwidgets.function->setEditText("");
            return;
         }
         auto qmi = this->_model->scriptQMI(name);
         if (qmi != none_qmi) {
            this->_subwidgets.function->setRootModelIndex(qmi);
            return;
         }
         //
         // The user typed a custom scriptname in.
         //
         this->_model->setUserItemScriptname(name);
         this->_subwidgets.function->setRootModelIndex(this->_model->userItemQMI());
      });
      QObject::connect(this->_subwidgets.scriptname, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
         if (i < 0) {
            this->_subwidgets.function->setRootModelIndex(this->_model->noneScriptQMI());
         } else {
            auto qmi = this->_model->index(i, 0, {});
            if (qmi.isValid()) {
               this->_subwidgets.function->setRootModelIndex(qmi);
            } else {
               this->_subwidgets.function->setRootModelIndex(this->_model->noneScriptQMI());
               this->_subwidgets.function->setEditText("");
            }
         }
         emit currentScriptnameChanged(this->_subwidgets.scriptname->currentText());
      });
      QObject::connect(this->_subwidgets.scriptname, &QComboBox::currentTextChanged, this, [this](const QString& name) {
         emit currentScriptnameChanged(name);
      });
      QObject::connect(this->_subwidgets.function, &QComboBox::currentTextChanged, this, [this](const QString& name) {
         emit currentFunctionChanged(name);
      });
   #endif
}

#if !defined(QT_PLUGIN)
QString DKPapyrusFragmentFunctionPicker::currentScriptname() const noexcept {
   return this->_subwidgets.scriptname->currentText();
}
QString DKPapyrusFragmentFunctionPicker::currentFunction() const noexcept {
   return this->_subwidgets.function->currentText();
}
void DKPapyrusFragmentFunctionPicker::setCurrentScriptname(const QString& value) {
   this->_subwidgets.scriptname->setCurrentText(value);
}
void DKPapyrusFragmentFunctionPicker::setCurrentScriptname(const std::string_view value) {
   this->_subwidgets.scriptname->setCurrentText(QString::fromUtf8(QByteArray(value.data(), value.size())));
}
void DKPapyrusFragmentFunctionPicker::setCurrentFunction(const QString& value) {
   this->_subwidgets.function->setCurrentText(value);
}
void DKPapyrusFragmentFunctionPicker::setCurrentFunction(const std::string_view value) {
   this->_subwidgets.function->setCurrentText(QString::fromUtf8(QByteArray(value.data(), value.size())));
}
#endif

QString DKPapyrusFragmentFunctionPicker::headerText() const {
   return this->_subwidgets.header->text();
}
void DKPapyrusFragmentFunctionPicker::setHeaderText(QString v) {
   this->_subwidgets.header->setText(v);
}

void DKPapyrusFragmentFunctionPicker::setSourceWidget(DKPapyrusBoundScriptListPane* src) {
   #if !defined(QT_PLUGIN)
      this->_model->setSourceWidget(src);
   #else
      this->_source_widget = src;
   #endif
}