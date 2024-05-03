#include "../widgets/DKCompactObjectReferencePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKCompactObjectReferencePickerInterface.h"

DKCompactObjectReferencePickerInterface::DKCompactObjectReferencePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKCompactObjectReferencePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKCompactObjectReferencePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKCompactObjectReferencePickerInterface::createWidget(QWidget* parent) {
   return new DKCompactObjectReferencePicker(parent);
}

QString DKCompactObjectReferencePickerInterface::name() const {
   return "DKCompactObjectReferencePicker";
}

QString DKCompactObjectReferencePickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKCompactObjectReferencePickerInterface::icon() const {
   return QIcon();
}

QString DKCompactObjectReferencePickerInterface::toolTip() const {
   return tr("A widget that can be used to select an ObjectReference (REFR form) through multiple means.");
}

QString DKCompactObjectReferencePickerInterface::whatsThis() const {
   return tr("A button that, when clicked, opens a dialog to select an ObjectReference.");
}

bool DKCompactObjectReferencePickerInterface::isContainer() const {
   return false;
}

QString DKCompactObjectReferencePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKCompactObjectReferencePicker" name="refPicker">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKCompactObjectReferencePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKCompactObjectReferencePickerInterface::includeFile() const {
   return "widgets/DKCompactObjectReferencePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
