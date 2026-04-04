#include "../widgets/DKPapyrusFragmentFunctionNamePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKPapyrusFragmentFunctionNamePickerInterface.h"

DKPapyrusFragmentFunctionNamePickerInterface::DKPapyrusFragmentFunctionNamePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKPapyrusFragmentFunctionNamePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKPapyrusFragmentFunctionNamePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKPapyrusFragmentFunctionNamePickerInterface::createWidget(QWidget* parent) {
   return new DKPapyrusFragmentFunctionNamePicker(parent);
}

QString DKPapyrusFragmentFunctionNamePickerInterface::name() const {
   return "DKPapyrusFragmentFunctionNamePicker";
}

QString DKPapyrusFragmentFunctionNamePickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKPapyrusFragmentFunctionNamePickerInterface::icon() const {
   return QIcon(":/icons/dropdown.png");
}

QString DKPapyrusFragmentFunctionNamePickerInterface::toolTip() const {
   return tr("A widget that can be used to set a Papyrus script function for use with fragments. Use when all fragments on a form must have the same scriptname.");
}

QString DKPapyrusFragmentFunctionNamePickerInterface::whatsThis() const {
   return tr("A widget that can be used to set a Papyrus script function for use with fragments. Use when all fragments on a form must have the same scriptname.");
}

bool DKPapyrusFragmentFunctionNamePickerInterface::isContainer() const {
   return false;
}

QString DKPapyrusFragmentFunctionNamePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKPapyrusFragmentFunctionNamePicker" name="fragmentFunction">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKPapyrusFragmentFunctionNamePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKPapyrusFragmentFunctionNamePickerInterface::includeFile() const {
   return "widgets/DKPapyrusFragmentFunctionNamePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
