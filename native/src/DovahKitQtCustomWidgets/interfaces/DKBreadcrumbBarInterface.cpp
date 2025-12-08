#include "../widgets/DKBreadcrumbBar.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKBreadcrumbBarInterface.h"

DKBreadcrumbBarInterface::DKBreadcrumbBarInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKBreadcrumbBarInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKBreadcrumbBarInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKBreadcrumbBarInterface::createWidget(QWidget* parent) {
   return new DKBreadcrumbBar(parent);
}

QString DKBreadcrumbBarInterface::name() const {
   return "DKBreadcrumbBar";
}

QString DKBreadcrumbBarInterface::group() const {
   return "DovahKit: General";
}

QIcon DKBreadcrumbBarInterface::icon() const {
   return QIcon();
}

QString DKBreadcrumbBarInterface::toolTip() const {
   return tr("A navigation bar for breadcrumbs.");
}

QString DKBreadcrumbBarInterface::whatsThis() const {
   return QString();
}

bool DKBreadcrumbBarInterface::isContainer() const {
   return true;
}

QString DKBreadcrumbBarInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKBreadcrumbBar" name="breadcrumbs">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKBreadcrumbBar</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKBreadcrumbBarInterface::includeFile() const {
   return "widgets/DKBreadcrumbBar.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
