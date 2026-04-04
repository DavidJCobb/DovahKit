#include "../widgets/DKPapyrusFragmentFunctionPicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKPapyrusFragmentFunctionPickerInterface.h"

DKPapyrusFragmentFunctionPickerInterface::DKPapyrusFragmentFunctionPickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKPapyrusFragmentFunctionPickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKPapyrusFragmentFunctionPickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKPapyrusFragmentFunctionPickerInterface::createWidget(QWidget* parent) {
   return new DKPapyrusFragmentFunctionPicker(parent);
}

QString DKPapyrusFragmentFunctionPickerInterface::name() const {
   return "DKPapyrusFragmentFunctionPicker";
}

QString DKPapyrusFragmentFunctionPickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKPapyrusFragmentFunctionPickerInterface::icon() const {
   return QIcon();
}

QString DKPapyrusFragmentFunctionPickerInterface::toolTip() const {
   return tr("A widget that can be used to set a Papyrus script and function for use as a Papyrus fragment.");
}

QString DKPapyrusFragmentFunctionPickerInterface::whatsThis() const {
   return tr("A widget that can be used to set a Papyrus script and function for use as a Papyrus fragment.");
}

bool DKPapyrusFragmentFunctionPickerInterface::isContainer() const {
   return false;
}

QString DKPapyrusFragmentFunctionPickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKPapyrusFragmentFunctionPicker" name="fragment">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKPapyrusFragmentFunctionPicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKPapyrusFragmentFunctionPickerInterface::includeFile() const {
   return "widgets/DKPapyrusFragmentFunctionPicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
