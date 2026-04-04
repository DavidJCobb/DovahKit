#include "../widgets/DKPapyrusFragmentScriptNamePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKPapyrusFragmentScriptNamePickerInterface.h"

DKPapyrusFragmentScriptNamePickerInterface::DKPapyrusFragmentScriptNamePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKPapyrusFragmentScriptNamePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKPapyrusFragmentScriptNamePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKPapyrusFragmentScriptNamePickerInterface::createWidget(QWidget* parent) {
   return new DKPapyrusFragmentScriptNamePicker(parent);
}

QString DKPapyrusFragmentScriptNamePickerInterface::name() const {
   return "DKPapyrusFragmentScriptNamePicker";
}

QString DKPapyrusFragmentScriptNamePickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKPapyrusFragmentScriptNamePickerInterface::icon() const {
   return QIcon(":/icons/dropdown.png");
}

QString DKPapyrusFragmentScriptNamePickerInterface::toolTip() const {
   return tr("A widget that can be used to set a Papyrus script for use with fragments. Use when all fragments on a form must have the same scriptname; then use individual function-name pickers for those fragments.");
}

QString DKPapyrusFragmentScriptNamePickerInterface::whatsThis() const {
   return tr("A widget that can be used to set a Papyrus script for use with fragments. Use when all fragments on a form must have the same scriptname; then use individual function-name pickers for those fragments.");
}

bool DKPapyrusFragmentScriptNamePickerInterface::isContainer() const {
   return false;
}

QString DKPapyrusFragmentScriptNamePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKPapyrusFragmentScriptNamePicker" name="fragmentScriptname">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKPapyrusFragmentScriptNamePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKPapyrusFragmentScriptNamePickerInterface::includeFile() const {
   return "widgets/DKPapyrusFragmentScriptNamePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
