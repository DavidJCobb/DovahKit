#include "../widgets/DKObjectReferencePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKObjectReferencePickerInterface.h"

DKObjectReferencePickerInterface::DKObjectReferencePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKObjectReferencePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKObjectReferencePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKObjectReferencePickerInterface::createWidget(QWidget* parent) {
   return new DKObjectReferencePicker(parent);
}

QString DKObjectReferencePickerInterface::name() const {
   return "DKObjectReferencePicker";
}

QString DKObjectReferencePickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKObjectReferencePickerInterface::icon() const {
   return QIcon();
}

QString DKObjectReferencePickerInterface::toolTip() const {
   return tr("A widget that can be used to select an ObjectReference (REFR form) through multiple means.");
}

QString DKObjectReferencePickerInterface::whatsThis() const {
   return tr("This widget consists, at minimum, of drop-down menus that can be used to select an ObjectReference in the game world. Optionally, the list of refs can be made filterable, and a button can be shown to visually select a ref from the Render Window.");
}

bool DKObjectReferencePickerInterface::isContainer() const {
   return false;
}

QString DKObjectReferencePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKObjectReferencePicker" name="refPicker">
      <property name="geometry">
         <rect>
            <x>0</x>
            <y>0</y>
            <width>100</width>
         </rect>
      </property>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKObjectReferencePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKObjectReferencePickerInterface::includeFile() const {
   return "widgets/DKObjectReferencePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
