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
   return "Containers";
}

QIcon DKCollapsiblePaneInterface::icon() const {
   return QIcon();
}

QString DKCollapsiblePaneInterface::toolTip() const {
   return QString();
}

QString DKCollapsiblePaneInterface::whatsThis() const {
   return QString();
}

bool DKCollapsiblePaneInterface::isContainer() const {
   return true;
}

QString DKCollapsiblePaneInterface::domXml() const {
   return "<widget class=\"DKCollapsiblePane\" name=\"collapsiblePane\">\n"
      " <property name=\"geometry\">\n"
      "  <rect>\n"
      "   <x>0</x>\n"
      "   <y>0</y>\n"
      "   <width>100</width>\n"
      "   <height>100</height>\n"
      "  </rect>\n"
      " </property>\n"
      " <property name=\"title\">\n"
      "  <string>Collapsible panel</string>\n"
      " </property>\n"
      " <widget class=\"QWidget\" name=\"collapsiblePaneWidgetContents\">\n" // this is how you supply a default page. we're not allowed to use our own page, but Qt Designer will create this and substitute it in.
      " </widget>"
      "</widget>\n";
}

QString DKCollapsiblePaneInterface::includeFile() const {
   return "widgets/DKCollapsiblePane.h";
}
