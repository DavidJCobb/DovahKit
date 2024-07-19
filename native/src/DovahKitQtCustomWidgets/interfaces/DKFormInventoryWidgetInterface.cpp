#include "../widgets/DKFormInventoryWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFormInventoryWidgetInterface.h"

DKFormInventoryWidgetInterface::DKFormInventoryWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFormInventoryWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFormInventoryWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFormInventoryWidgetInterface::createWidget(QWidget* parent) {
   return new DKFormInventoryWidget(parent);
}

QString DKFormInventoryWidgetInterface::name() const {
   return "DKFormInventoryWidget";
}

QString DKFormInventoryWidgetInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKFormInventoryWidgetInterface::icon() const {
   return QIcon();
}

QString DKFormInventoryWidgetInterface::toolTip() const {
   return tr("A pane that can be used to edit a container's initial inventory.");
}

QString DKFormInventoryWidgetInterface::whatsThis() const {
   return QString();
}

bool DKFormInventoryWidgetInterface::isContainer() const {
   return false;
}

QString DKFormInventoryWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFormInventoryWidget" name="inventory">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFormInventoryWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFormInventoryWidgetInterface::includeFile() const {
   return "widgets/DKFormInventoryWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
