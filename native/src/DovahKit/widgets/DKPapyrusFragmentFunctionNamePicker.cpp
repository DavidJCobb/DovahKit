#include "./DKPapyrusFragmentFunctionNamePicker.h"
#include <QGridLayout>
#include <QLineEdit>
#if !defined(QT_PLUGIN)
   #include "./widget-models/DKPapyrusFragmentFunctionModel.h"
#endif

DKPapyrusFragmentFunctionNamePicker::DKPapyrusFragmentFunctionNamePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   {
      auto* widget = this->_subwidgets.function = new QComboBox(this);
      layout->addWidget(widget, 0, 0);

      widget->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
      widget->setEditable(true);
      widget->lineEdit()->setMaxLength(maxFunctionNameLength);
   }
   layout->setColumnStretch(0, 1);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.function);

   #if !defined(QT_PLUGIN)
      this->_model = new DKPapyrusFragmentFunctionModel(this);

      this->_subwidgets.function->setModel(this->_model);
      this->_subwidgets.function->setRootModelIndex(this->_model->noneScriptQMI());

      QObject::connect(this->_subwidgets.function, &QComboBox::currentTextChanged, this, [this](const QString& name) {
         emit valueChanged(name);
      });
   #endif
}

QString DKPapyrusFragmentFunctionNamePicker::value() const noexcept {
   return this->_subwidgets.function->currentText();
}
void DKPapyrusFragmentFunctionNamePicker::setValue(const QString& value) {
   this->_subwidgets.function->setCurrentText(value);
}
void DKPapyrusFragmentFunctionNamePicker::setValue(const std::string_view value) {
   this->_subwidgets.function->setCurrentText(QString::fromUtf8(QByteArray(value.data(), value.size())));
}

void DKPapyrusFragmentFunctionNamePicker::setSourceWidget(DKPapyrusFragmentScriptNamePicker* src) {
   if (src == this->_source)
      return;
   if (this->_source)
      QObject::disconnect(this->_source, nullptr, this, nullptr);

   this->_source = src;
   DKPapyrusBoundScriptListPane* script_list = nullptr;
   if (src) {
      script_list = src->sourceWidget();
      #if !defined(QT_PLUGIN)
         QObject::connect(this->_source, &DKPapyrusFragmentScriptNamePicker::valueChanged, this, [this](QString name) {
            const auto  none_qmi = this->_model->noneScriptQMI();
            if (name.isEmpty()) {
               this->_subwidgets.function->setRootModelIndex(none_qmi);
               return;
            }
            QModelIndex qmi = this->_model->scriptQMI(name);
            if (qmi != none_qmi) {
               this->_subwidgets.function->setRootModelIndex(qmi);
            } else {
               this->_model->setUserItemScriptname(name);
               this->_subwidgets.function->setRootModelIndex(this->_model->userItemQMI());
            }
         });
      #endif
   }
   #if !defined(QT_PLUGIN)
      this->_model->setSourceWidget(script_list);
   #endif
}