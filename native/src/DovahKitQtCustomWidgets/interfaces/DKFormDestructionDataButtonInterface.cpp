#include "../widgets/DKFormDestructionDataButton.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFormDestructionDataButtonInterface.h"

DKFormDestructionDataButtonInterface::DKFormDestructionDataButtonInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFormDestructionDataButtonInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFormDestructionDataButtonInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFormDestructionDataButtonInterface::createWidget(QWidget* parent) {
   return new DKFormDestructionDataButton(parent);
}

QString DKFormDestructionDataButtonInterface::name() const {
   return "DKFormDestructionDataButton";
}

QString DKFormDestructionDataButtonInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKFormDestructionDataButtonInterface::icon() const {
   return QIcon();
}

QString DKFormDestructionDataButtonInterface::toolTip() const {
   return tr("A button that, when clicked, allows editing of a form's destruction stage data.");
}

QString DKFormDestructionDataButtonInterface::whatsThis() const {
   return QString();
}

bool DKFormDestructionDataButtonInterface::isContainer() const {
   return false;
}

QString DKFormDestructionDataButtonInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFormDestructionDataButton" name="destructionData">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFormDestructionDataButton</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFormDestructionDataButtonInterface::includeFile() const {
   return "widgets/DKFormDestructionDataButton.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
