#include "../widgets/DKTabWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKTabWidgetInterface.h"

DKTabWidgetInterface::DKTabWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKTabWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKTabWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKTabWidgetInterface::createWidget(QWidget* parent) {
   return new DKTabWidget(parent);
}

QString DKTabWidgetInterface::name() const {
   return "DKTabWidget";
}

QString DKTabWidgetInterface::group() const {
   return "DovahKit: Extended Qt Widgets";
}

QIcon DKTabWidgetInterface::icon() const {
   return QIcon(":/widgets/tabwidget.png"); // blind guess: can we use Qt Designer's own widget icons?
}

QString DKTabWidgetInterface::toolTip() const {
   return tr("An alternative to QTabWidget with bugfixes.");
}

QString DKTabWidgetInterface::whatsThis() const {
   return tr("An alternative to QTabWidget with bugfixes.");
}

bool DKTabWidgetInterface::isContainer() const {
   return true;
}

QString DKTabWidgetInterface::domXml() const {
   //
   // NOTES:
   // 
   // DO NOT add an `addpagemethod` to the custom widget definition. Doing so will 
   // prevent Qt Designer from using the hardcoded special-case handling that it 
   // uses to support QTabWidget (i.e. passing a dummy QString as the tab name when 
   // adding the tab, and then setting the tab name properly later on).
   //
   return R"555(
<ui language="c++">
   <widget class="DKTabWidget" name="tabWidget">
      <property name="currentIndex">
         <number>0</number>
      </property>
      <widget class="QWidget" name="tab">
         <attribute name="title">
            <string>Tab 1</string>
         </attribute>
      </widget>
      <widget class="QWidget" name="tab">
         <attribute name="title">
            <string>Tab 2</string>
         </attribute>
      </widget>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKTabWidget</class>
         <extends>QTabWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKTabWidgetInterface::includeFile() const {
   return "widgets/DKTabWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
