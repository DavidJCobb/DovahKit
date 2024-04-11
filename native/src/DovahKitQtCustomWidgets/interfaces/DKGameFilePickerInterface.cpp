#include "../widgets/DKGameFilePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKGameFilePickerInterface.h"

DKGameFilePickerInterface::DKGameFilePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKGameFilePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKGameFilePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKGameFilePickerInterface::createWidget(QWidget* parent) {
   return new DKGameFilePicker(parent);
}

QString DKGameFilePickerInterface::name() const {
   return "DKGameFilePicker";
}

QString DKGameFilePickerInterface::group() const {
   return "DovahKit";
}

QIcon DKGameFilePickerInterface::icon() const {
   return QIcon();
}

QString DKGameFilePickerInterface::toolTip() const {
   return tr("A widget that can be used to select a game asset path.");
}

QString DKGameFilePickerInterface::whatsThis() const {
   return tr("This widget consists of a textbox and a browse button. Clicking the button opens a custom file picker capable of browsing the contents of loaded BSAs (Bethesda Softworks Archives).");
}

bool DKGameFilePickerInterface::isContainer() const {
   return false;
}

QString DKGameFilePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKGameFilePicker" name="filePicker">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKGameFilePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKGameFilePickerInterface::includeFile() const {
   return "widgets/DKGameFilePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
