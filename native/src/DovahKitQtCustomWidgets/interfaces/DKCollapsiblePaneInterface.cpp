#include "../widgets/DKCollapsiblePane.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKCollapsiblePaneInterface.h"
#include "../extensions/DKCollapsiblePaneContainerExtension.h"
#include "../extensions/DKCollapsiblePaneTaskMenuExtension.h"
#include "../extension_factories/DKCollapsiblePaneExtensionFactory.h"

DKCollapsiblePaneInterface::DKCollapsiblePaneInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKCollapsiblePaneInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   QExtensionFactory* factory = new DKCollapsiblePaneExtensionFactory(manager);
   assert(manager);
   manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
   manager->registerExtensions(factory, Q_TYPEID(QDesignerTaskMenuExtension));
   //
   this->initialized = true;
}

bool DKCollapsiblePaneInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKCollapsiblePaneInterface::createWidget(QWidget* parent) {
   auto* out = new DKCollapsiblePane(parent);
   if (auto* body = out->viewport()) {
      auto name = out->objectName();
      if (name.isEmpty())
         name = this->name();
      name = QString("%1_viewport").arg(name);
      body->setObjectName(name);
   }
   return out;
}

QString DKCollapsiblePaneInterface::name() const {
   return "DKCollapsiblePane";
}

QString DKCollapsiblePaneInterface::group() const {
   return "DovahKit: General";
}

QIcon DKCollapsiblePaneInterface::icon() const {
   return QIcon();
}

QString DKCollapsiblePaneInterface::toolTip() const {
   return tr("A panel with a button to show or hide the content.");
}

QString DKCollapsiblePaneInterface::whatsThis() const {
   return QString();
}

bool DKCollapsiblePaneInterface::isContainer() const {
   return true;
}

QString DKCollapsiblePaneInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKCollapsiblePane" name="collapsiblePane">
      <property name="geometry">
         <rect>
            <x>0</x>
            <y>0</y>
            <width>100</width>
            <height>100</height>
         </rect>
      </property>
      <property name="title">
         <string>Collapsible panel</string>
      </property>
      <widget class="QWidget" name="collapsiblePaneWidgetContents">
      </widget>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKCollapsiblePane</class>
         <extends>QFrame</extends>
         <addpagemethod>setViewport</addpagemethod>
      </customwidget>
   </customwidgets>
</ui>
)555";
   //
   // Define an initial page by specifying a child widget, and use the customwidgets tag 
   // to ensure that Qt's build system knows how to add it as a page. Because our widget 
   // uses the "container" extension, Qt Designer will do the rest.
   //
}

QString DKCollapsiblePaneInterface::includeFile() const {
   return "widgets/DKCollapsiblePane.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
