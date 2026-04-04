#include "./DKPapyrusFragmentScriptNamePicker.h"
#include <QGridLayout>
#include <QLineEdit>

DKPapyrusFragmentScriptNamePicker::DKPapyrusFragmentScriptNamePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   {
      auto* widget = this->_subwidgets.scriptname = new QComboBox(this);
      layout->addWidget(widget, 0, 0);

      widget->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
      widget->setEditable(true);
      widget->lineEdit()->setMaxLength(maxScriptnameLength);
   }
   layout->setColumnStretch(0, 1);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.scriptname);

   QObject::connect(this->_subwidgets.scriptname, &QComboBox::editTextChanged, this, [this](const QString& name) {
      emit valueChanged(this->value());
   });
   QObject::connect(this->_subwidgets.scriptname, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      emit valueChanged(this->value());
   });
   QObject::connect(this->_subwidgets.scriptname, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      emit valueChanged(name);
   });
}

QString DKPapyrusFragmentScriptNamePicker::value() const noexcept {
   return this->_subwidgets.scriptname->currentText();
}
void DKPapyrusFragmentScriptNamePicker::setValue(const QString& value) {
   this->_subwidgets.scriptname->setCurrentText(value);
}
void DKPapyrusFragmentScriptNamePicker::setValue(const std::string_view value) {
   this->_subwidgets.scriptname->setCurrentText(QString::fromUtf8(QByteArray(value.data(), value.size())));
}

void DKPapyrusFragmentScriptNamePicker::setSourceWidget(DKPapyrusBoundScriptListPane* src) {
   if (this->_source == src)
      return;

   #if !defined(QT_PLUGIN)
      QString prior_name       = this->value();
      bool    prior_was_custom = true;
      if (this->_source) {
         QObject::disconnect(this->_source, nullptr, this, nullptr);
         prior_was_custom = this->_source->hasScript(prior_name, false);
      }
   #endif
   this->_source = src;

   #if !defined(QT_PLUGIN)
      auto*      widget  = this->_subwidgets.scriptname;
      const auto blocker = QSignalBlocker(widget);
      widget->clear();
      if (src) {
         QObject::connect(this->_source, &DKPapyrusBoundScriptListPane::scriptAdded,   this, &DKPapyrusFragmentScriptNamePicker::_on_source_script_list_changed);
         QObject::connect(this->_source, &DKPapyrusBoundScriptListPane::scriptRemoved, this, &DKPapyrusFragmentScriptNamePicker::_on_source_script_list_changed);
      
         auto list = this->_source->allNonDeletedScriptnames();
         for (auto& item : list)
            widget->addItem(item);
      }
      if (prior_was_custom) {
         widget->setCurrentText(prior_name);
      } else {
         widget->setCurrentText("");
         if (!prior_name.isEmpty()) {
            emit valueChanged(this->value());
         }
      }
   #endif

   emit sourceWidgetChanged(this->_source);
}

void DKPapyrusFragmentScriptNamePicker::_on_source_script_list_changed() {
   #if !defined(QT_PLUGIN)
      auto*      widget  = this->_subwidgets.scriptname;
      const auto blocker = QSignalBlocker(widget);

      int  prior_index = widget->currentIndex();
      auto prior_text  = widget->currentText();
      bool prior_keep  = prior_index < 0;
      if (this->_source) {
         auto list = this->_source->allNonDeletedScriptnames();
         for (auto& item : list) {
            widget->addItem(item);
            if (!prior_keep) {
               prior_keep = item == prior_text;
            }
         }
      }
      if (prior_keep) {
         widget->setCurrentText(prior_text);
      } else {
         widget->setCurrentIndex(-1);
         widget->setCurrentText("");
         if (!prior_text.isEmpty()) {
            emit valueChanged(this->value());
         }
      }
   #endif
}