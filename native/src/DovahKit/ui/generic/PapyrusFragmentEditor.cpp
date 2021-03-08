#include "PapyrusFragmentEditor.h"
#include "../../editor/papyrus.h"
#include "../../dovah/files/papyrus/compiled_script.h"

PapyrusFragmentEditor::PapyrusFragmentEditor(QWidget* parent) : QWidget(parent) {
   this->ui.setupUi(this);
   //
   QObject::connect(this->ui.scriptname, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      auto* widget = this->ui.function;
      auto  count  = widget->count();
      for (int i = 0; i < count; ++i) {
         auto data = widget->itemData(i, Qt::UserRole);
         if (data.type() == QMetaType::Bool && data.toBool()) {
            widget->removeItem(i);
            --i;
            --count;
         }
      }
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
                  widget->addItem(function.name.c_str(), true);
               }
            }
         }
      }
      emit currentScriptnameChanged(name);
   });
   QObject::connect(this->ui.function, &QComboBox::currentTextChanged, this, [this](const QString& name) {
      emit currentFunctionChanged(name);
   });
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

void PapyrusFragmentEditor::addScriptname(const QString& scriptname) {
   if (this->_getScriptData(scriptname)) {
      //
      // The item could still have been typed in by the user; flag it as predefined.
      //
      auto* widget = this->ui.function;
      auto  count  = widget->count();
      for (int i = 0; i < count; ++i) {
         auto data = widget->itemData(i, Qt::UserRole);
         if (data.type() == QMetaType::Bool && !data.toBool()) {
            data = widget->itemData(i, Qt::DisplayRole);
            if (scriptname.compare(data.toString(), Qt::CaseInsensitive) == 0) {
               widget->setItemData(i, true, Qt::UserRole);
               break;
            }
         }
      }
      return;
   }
   auto* data = DovahKitPapyrusDictionary::get().get_script_for(this, scriptname);
   if (data) {
      script entry;
      entry.name     = scriptname;
      entry.compiled = data;
      this->scripts.append(entry);
   }
   this->ui.scriptname->addItem(scriptname, true);
}
void PapyrusFragmentEditor::clearAvailableScriptnames() {
   auto* widget = this->ui.scriptname;
   int   count  = widget->count();
   for (int i = 0; i < count; ++i) {
      auto data = widget->itemData(i, Qt::UserRole);
      if (data.type() == QMetaType::Bool && data.toBool()) {
         widget->removeItem(i);
         --i;
         --count;
      }
   }
   this->scripts.clear();
}
void PapyrusFragmentEditor::clearCurrentValues() {
   this->ui.scriptname->clearEditText();
   this->ui.function->clearEditText();
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