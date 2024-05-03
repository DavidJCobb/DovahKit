#include "./DKPapyrusFragmentFunctionPicker.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#if !defined(QT_DESIGNER_LIB)
   #include <QSortFilterProxyModel>
   #include <QStandardItemModel>
   #include "dovah/files/papyrus/compiled_script.h"
   #include "editor/papyrus_dictionary.h"
#endif

#if !defined(QT_DESIGNER_LIB)
namespace {
   inline void _append_combobox_item(QComboBox* widget, QStandardItem* item) {
      auto* proxy = (QSortFilterProxyModel*) widget->model();
      auto* model = (QStandardItemModel*) proxy->sourceModel();
      model->appendRow(item);
   }
   inline void _append_combobox_item(QComboBox* widget, const QString& text, const QVariant& userdata = QVariant()) {
      auto* proxy = (QSortFilterProxyModel*) widget->model();
      auto* model = (QStandardItemModel*) proxy->sourceModel();
      auto* item  = new QStandardItem(text);
      item->setData(userdata, Qt::UserRole);
      model->appendRow(item);
   }
}
#endif

DKPapyrusFragmentFunctionPicker::DKPapyrusFragmentFunctionPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   layout->setContentsMargins(0, 0, 0, 0);

   {
      auto* label = new QLabel(tr("Script fragment data"), this);
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

   #if !defined(QT_DESIGNER_LIB)
   QObject::connect(this->_subwidgets.scriptname, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      auto* widget = this->_subwidgets.function;
      widget->clear();
      //
      auto* entry = this->_getOrCreateScriptData(name);
      if (entry) {
         auto* compiled = entry->compiled;
         if (compiled) {
            for (auto& object : compiled->objects) {
               if (name.compare(object.name.c_str(), Qt::CaseInsensitive) != 0)
                  continue;
               auto* state = object.get_auto_state();
               if (!state)
                  continue;
               for (auto& function : state->functions) {
                  if (!function.arguments.empty())
                     continue;
                  if (function.flags & dovah::compiled_papyrus_script::function::flag::global)
                     continue;
                  if (function.name == "GetState") // apparently this isn't hardcoded, then?
                     continue;
                  // QComboBox::addItem seems to break when using a QSortFilterProxyModel, so we have to do it ourselves...
                  _append_combobox_item(widget, function.name.c_str(), true);
               }
            }
         }
      }
      emit currentScriptnameChanged(name);
   });
   QObject::connect(this->_subwidgets.function, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      emit currentFunctionChanged(name);
   });
   #endif

   #if !defined(QT_DESIGNER_LIB)
   #pragma region sorting
   {
      auto* widget = this->_subwidgets.scriptname;
      auto* model  = new QStandardItemModel(widget); // can't reuse the original model; QComboBox kills it when you call setModel
      auto* proxy  = new QSortFilterProxyModel(widget);
      proxy->setSourceModel(model);
      widget->setModel(proxy);
      proxy->sort(0);
   }
   {
      auto* widget = this->_subwidgets.function;
      auto* model  = new QStandardItemModel(widget); // can't reuse the original model; QComboBox kills it when you call setModel
      auto* proxy  = new QSortFilterProxyModel(widget);
      proxy->setSourceModel(model);
      widget->setModel(proxy);
      proxy->sort(0);
   }
   #pragma endregion
   #endif
}

#if !defined(QT_DESIGNER_LIB)
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

int DKPapyrusFragmentFunctionPicker::scriptnameMaxLength() const noexcept {
   if (auto* line = this->_subwidgets.scriptname->lineEdit())
      return line->maxLength();
   return -1;
}
void DKPapyrusFragmentFunctionPicker::setScriptnameMaxLength(size_t s) noexcept {
   if (auto* line = this->_subwidgets.scriptname->lineEdit())
      line->setMaxLength(s);
}
int DKPapyrusFragmentFunctionPicker::functionMaxLength() const noexcept {
   if (auto* line = this->_subwidgets.function->lineEdit())
      return line->maxLength();
   return -1;
}
void DKPapyrusFragmentFunctionPicker::setFunctionMaxLength(size_t s) noexcept {
   if (auto* line = this->_subwidgets.function->lineEdit())
      line->setMaxLength(s);
}

void DKPapyrusFragmentFunctionPicker::addScriptname(const QString& scriptname) {
   if (this->_getScriptData(scriptname)) {
      _append_combobox_item(this->_subwidgets.scriptname, scriptname, true);
      return;
   }
   auto* data = DovahKitPapyrusDictionary::get().get_script_for(this, scriptname);
   if (data) {
      script entry;
      entry.name     = scriptname;
      entry.compiled = data;
      this->scripts.append(entry);
   }
   // QComboBox::addItem seems to break when using a QSortFilterProxyModel, so we have to do it ourselves...
   _append_combobox_item(this->_subwidgets.scriptname, scriptname, true);
}
void DKPapyrusFragmentFunctionPicker::clearAvailableScriptnames() {
   this->_subwidgets.scriptname->clear();
   //
   auto& dictionary = DovahKitPapyrusDictionary::get();
   for (auto& script : this->scripts)
      dictionary.relinquish_script_from(this, script.name);
   this->scripts.clear();
}
void DKPapyrusFragmentFunctionPicker::clearCurrentValues() {
   this->_subwidgets.scriptname->clearEditText();
   this->_subwidgets.function->clearEditText();
}
void DKPapyrusFragmentFunctionPicker::removeScriptname(const QString& name) {
   auto& list = this->scripts;
   for (auto it = list.begin(); it != list.end(); ++it) {
      if (it->name == name) {
         list.erase(it);
         DovahKitPapyrusDictionary::get().relinquish_script_from(this, name);
         return;
      }
   }
}

DKPapyrusFragmentFunctionPicker::script* DKPapyrusFragmentFunctionPicker::_getScriptData(const QString& name) {
   if (name.isEmpty())
      return nullptr;
   for (auto& entry : this->scripts) {
      if (entry.name == name)
         return &entry;
   }
   return nullptr;
}
DKPapyrusFragmentFunctionPicker::script* DKPapyrusFragmentFunctionPicker::_getOrCreateScriptData(const QString& name) {
   if (name.isEmpty())
      return nullptr;
   for (auto& entry : this->scripts)
      if (entry.name == name)
         return &entry;
   auto* data = DovahKitPapyrusDictionary::get().get_script_for(this, name);
   if (!data)
      return nullptr;
   script entry;
   entry.name     = name;
   entry.compiled = data;
   this->scripts.append(entry);
   return &(this->scripts.back());
}
#endif