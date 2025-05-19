#include "../widgets/DKNavmeshGenerationImportOptionPicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKNavmeshGenerationImportOptionPickerInterface.h"

DKNavmeshGenerationImportOptionPickerInterface::DKNavmeshGenerationImportOptionPickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKNavmeshGenerationImportOptionPickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKNavmeshGenerationImportOptionPickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKNavmeshGenerationImportOptionPickerInterface::createWidget(QWidget* parent) {
   return new DKNavmeshGenerationImportOptionPicker(parent);
}

QString DKNavmeshGenerationImportOptionPickerInterface::name() const {
   return "DKNavmeshGenerationImportOptionPicker";
}

QString DKNavmeshGenerationImportOptionPickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKNavmeshGenerationImportOptionPickerInterface::icon() const {
   return QIcon(":/icons/dropdown.png");
}

QString DKNavmeshGenerationImportOptionPickerInterface::toolTip() const {
   return tr("A widget that can be used to set how a base form influences automatic navmesh generation.");
}

QString DKNavmeshGenerationImportOptionPickerInterface::whatsThis() const {
   return tr("A widget that can be used to set how a base form influences automatic navmesh generation.");
}

bool DKNavmeshGenerationImportOptionPickerInterface::isContainer() const {
   return false;
}

QString DKNavmeshGenerationImportOptionPickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKNavmeshGenerationImportOptionPicker" name="navmeshGeneration">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKNavmeshGenerationImportOptionPicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKNavmeshGenerationImportOptionPickerInterface::includeFile() const {
   return "widgets/DKNavmeshGenerationImportOptionPicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
