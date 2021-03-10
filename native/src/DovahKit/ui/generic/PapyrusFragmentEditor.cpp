#include "PapyrusFragmentEditor.h"
#include "../../editor/papyrus_dictionary.h"
#include "../../dovah/files/papyrus/compiled_script.h"
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

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

PapyrusFragmentEditor::PapyrusFragmentEditor(QWidget* parent) : QWidget(parent) {
   this->ui.setupUi(this);
   //
   QObject::connect(this->ui.scriptname, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      auto* widget = this->ui.function;
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
   QObject::connect(this->ui.function, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      emit currentFunctionChanged(name);
   });
   //
   #pragma region sorting
   {
      auto* widget = this->ui.scriptname;
      auto* model  = new QStandardItemModel(widget); // can't reuse the original model; QComboBox kills it when you call setModel
      auto* proxy  = new QSortFilterProxyModel(widget);
      proxy->setSourceModel(model);
      widget->setModel(proxy);
      proxy->sort(0);
   }
   {
      auto* widget = this->ui.function;
      auto* model  = new QStandardItemModel(widget); // can't reuse the original model; QComboBox kills it when you call setModel
      auto* proxy  = new QSortFilterProxyModel(widget);
      proxy->setSourceModel(model);
      widget->setModel(proxy);
      proxy->sort(0);
   }
   #pragma endregion
}

QString PapyrusFragmentEditor::currentScriptname() const noexcept {
   return this->ui.scriptname->currentText();
}
QString PapyrusFragmentEditor::currentFunction() const noexcept {
   return this->ui.function->currentText();
}
void PapyrusFragmentEditor::setCurrentScriptname(const QString& value) {
   this->ui.scriptname->setCurrentText(value);
}
void PapyrusFragmentEditor::setCurrentScriptname(const char* value) {
   this->ui.scriptname->setCurrentText(value);
}
void PapyrusFragmentEditor::setCurrentFunction(const QString& value) {
   this->ui.function->setCurrentText(value);
}
void PapyrusFragmentEditor::setCurrentFunction(const char* value) {
   this->ui.function->setCurrentText(value);
}

int PapyrusFragmentEditor::scriptnameMaxLength() const noexcept {
   if (auto* line = this->ui.scriptname->lineEdit())
      return line->maxLength();
   return -1;
}
void PapyrusFragmentEditor::setScriptnameMaxLength(int s) noexcept {
   if (auto* line = this->ui.scriptname->lineEdit())
      line->setMaxLength(s);
}
int PapyrusFragmentEditor::functionMaxLength() const noexcept {
   if (auto* line = this->ui.function->lineEdit())
      return line->maxLength();
   return -1;
}
void PapyrusFragmentEditor::setFunctionMaxLength(int s) noexcept {
   if (auto* line = this->ui.function->lineEdit())
      line->setMaxLength(s);
}

void PapyrusFragmentEditor::addScriptname(const QString& scriptname) {
   if (this->_getScriptData(scriptname)) {
      _append_combobox_item(this->ui.scriptname, scriptname, true);
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
   _append_combobox_item(this->ui.scriptname, scriptname, true);
}
void PapyrusFragmentEditor::clearAvailableScriptnames() {
   this->ui.scriptname->clear();
   //
   auto& dictionary = DovahKitPapyrusDictionary::get();
   for (auto& script : this->scripts)
      dictionary.relinquish_script_from(this, script.name);
   this->scripts.clear();
}
void PapyrusFragmentEditor::clearCurrentValues() {
   this->ui.scriptname->clearEditText();
   this->ui.function->clearEditText();
}
void PapyrusFragmentEditor::removeScriptname(const QString& name) {
   auto& list = this->scripts;
   for (auto it = list.begin(); it != list.end(); ++it) {
      if (it->name == name) {
         list.erase(it);
         DovahKitPapyrusDictionary::get().relinquish_script_from(this, name);
         return;
      }
   }
}

PapyrusFragmentEditor::script* PapyrusFragmentEditor::_getScriptData(const QString& name) {
   if (name.isEmpty())
      return nullptr;
   for (auto& entry : this->scripts) {
      if (entry.name == name)
         return &entry;
   }
   return nullptr;
}
PapyrusFragmentEditor::script* PapyrusFragmentEditor::_getOrCreateScriptData(const QString& name) {
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