#include "../widgets/DKFormNIFPicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFormNIFPickerInterface.h"

DKFormNIFPickerInterface::DKFormNIFPickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFormNIFPickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFormNIFPickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFormNIFPickerInterface::createWidget(QWidget* parent) {
   return new DKFormNIFPicker(parent);
}

QString DKFormNIFPickerInterface::name() const {
   return "DKFormNIFPicker";
}

QString DKFormNIFPickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKFormNIFPickerInterface::icon() const {
   return QIcon();
}

QString DKFormNIFPickerInterface::toolTip() const {
   return tr("A widget that can be used to select a NIF file to use for a form.");
}

QString DKFormNIFPickerInterface::whatsThis() const {
   return tr("This widget can be used to select a NIF file to use for a form. When applicable, texture-swap options are made available.");
}

bool DKFormNIFPickerInterface::isContainer() const {
   return false;
}

QString DKFormNIFPickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFormNIFPicker" name="nifPicker">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFormNIFPicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFormNIFPickerInterface::includeFile() const {
   return "widgets/DKFormNIFPicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
