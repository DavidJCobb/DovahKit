#include "../widgets/DKFormListPane.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFormListPaneInterface.h"

DKFormListPaneInterface::DKFormListPaneInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFormListPaneInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFormListPaneInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFormListPaneInterface::createWidget(QWidget* parent) {
   return new DKFormListPane(parent);
}

QString DKFormListPaneInterface::name() const {
   return "DKFormListPane";
}

QString DKFormListPaneInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKFormListPaneInterface::icon() const {
   return QIcon();
}

QString DKFormListPaneInterface::toolTip() const {
   return tr("A pane that can be used to edit a list of forms.");
}

QString DKFormListPaneInterface::whatsThis() const {
   return QString();
}

bool DKFormListPaneInterface::isContainer() const {
   return false;
}

QString DKFormListPaneInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFormListPane" name="formListPane">
      <property name="geometry">
         <rect>
            <x>0</x>
            <y>0</y>
            <width>100</width>
            <height>100</height>
         </rect>
      </property>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFormListPane</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFormListPaneInterface::includeFile() const {
   return "widgets/DKFormListPane.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
