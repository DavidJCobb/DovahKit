#include "../widgets/DKFormPicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFormPickerInterface.h"

DKFormPickerInterface::DKFormPickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFormPickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFormPickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFormPickerInterface::createWidget(QWidget* parent) {
   return new DKFormPicker(parent);
}

QString DKFormPickerInterface::name() const {
   return "DKFormPicker";
}

QString DKFormPickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKFormPickerInterface::icon() const {
   return QIcon();
}

QString DKFormPickerInterface::toolTip() const {
   return tr("A widget that can be used to select a form in Skyrim.");
}

QString DKFormPickerInterface::whatsThis() const {
   return tr("This widget can consist either of a single drop-down menu for forms, or for a drop-down to filter by form type and another drop-down to select a form.");
}

bool DKFormPickerInterface::isContainer() const {
   return false;
}

QString DKFormPickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFormPicker" name="formpicker">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFormPicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFormPickerInterface::includeFile() const {
   return "widgets/DKFormPicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
