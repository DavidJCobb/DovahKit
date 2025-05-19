#include "../widgets/DKYesNoUnsetWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKYesNoUnsetWidgetInterface.h"
#include "../extensions/DKYesNoUnsetWidgetTaskMenuExtension.h"
#include "../extension_factories/DKYesNoUnsetWidgetExtensionFactory.h"

DKYesNoUnsetWidgetInterface::DKYesNoUnsetWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKYesNoUnsetWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   QExtensionFactory* factory = new DKYesNoUnsetWidgetExtensionFactory(manager);
   assert(manager);
   manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));
   //
   this->initialized = true;
}

bool DKYesNoUnsetWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKYesNoUnsetWidgetInterface::createWidget(QWidget* parent) {
   auto* widget = new DKYesNoUnsetWidget(parent);
   widget->setText("Widget");
   return widget;
}

QString DKYesNoUnsetWidgetInterface::name() const {
   return "DKYesNoUnsetWidget";
}

QString DKYesNoUnsetWidgetInterface::group() const {
   return "DovahKit: Extended Qt Widgets";
}

QIcon DKYesNoUnsetWidgetInterface::icon() const {
   return QIcon(":/icons/DKYesNoUnsetWidget.png");
}

QString DKYesNoUnsetWidgetInterface::toolTip() const {
   return tr("A tri-state control allowing the user to indicate yes/no/unset values.");
}

QString DKYesNoUnsetWidgetInterface::whatsThis() const {
   return tr("A tri-state control allowing the user to indicate yes/no/unset values. The push-button controls whether the user is providing a value, and the checkbox controls the value provided.");
}

bool DKYesNoUnsetWidgetInterface::isContainer() const {
   return false;
}

QString DKYesNoUnsetWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKYesNoUnsetWidget" name="yesNoUnsetWidget">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKYesNoUnsetWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKYesNoUnsetWidgetInterface::includeFile() const {
   return "widgets/DKYesNoUnsetWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
