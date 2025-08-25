#include "./DKIdleCollectionWidgetInterface.h"
#include "../widgets/DKIdleCollectionWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

DKIdleCollectionWidgetInterface::DKIdleCollectionWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKIdleCollectionWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKIdleCollectionWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKIdleCollectionWidgetInterface::createWidget(QWidget* parent) {
   return new DKIdleCollectionWidget(parent);
}

QString DKIdleCollectionWidgetInterface::name() const {
   return "DKIdleCollectionWidget";
}

QString DKIdleCollectionWidgetInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKIdleCollectionWidgetInterface::icon() const {
   return QIcon();
}

QString DKIdleCollectionWidgetInterface::toolTip() const {
   return tr("A pane that can be used to edit the collection of idles seen in Idle Marker and Package forms.");
}

QString DKIdleCollectionWidgetInterface::whatsThis() const {
   return QString();
}

bool DKIdleCollectionWidgetInterface::isContainer() const {
   return false;
}

QString DKIdleCollectionWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKIdleCollectionWidget" name="idles">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKIdleCollectionWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKIdleCollectionWidgetInterface::includeFile() const {
   return "widgets/DKIdleCollectionWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
