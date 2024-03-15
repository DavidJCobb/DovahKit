#include "../widgets/DKPapyrusBoundScriptListPane.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKPapyrusBoundScriptListPaneInterface.h"

DKPapyrusBoundScriptListPaneInterface::DKPapyrusBoundScriptListPaneInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKPapyrusBoundScriptListPaneInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKPapyrusBoundScriptListPaneInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKPapyrusBoundScriptListPaneInterface::createWidget(QWidget* parent) {
   return new DKPapyrusBoundScriptListPane(parent);
}

QString DKPapyrusBoundScriptListPaneInterface::name() const {
   return "DKPapyrusBoundScriptListPane";
}

QString DKPapyrusBoundScriptListPaneInterface::group() const {
   return "DovahKit";
}

QIcon DKPapyrusBoundScriptListPaneInterface::icon() const {
   return QIcon();
}

QString DKPapyrusBoundScriptListPaneInterface::toolTip() const {
   return tr("A pane that can be used to edit the Papyrus scripts bound to a form or quest alias.");
}

QString DKPapyrusBoundScriptListPaneInterface::whatsThis() const {
   return QString();
}

bool DKPapyrusBoundScriptListPaneInterface::isContainer() const {
   return false;
}

QString DKPapyrusBoundScriptListPaneInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKPapyrusBoundScriptListPane" name="scriptListPane">
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

QString DKPapyrusBoundScriptListPaneInterface::includeFile() const {
   return "widgets/DKPapyrusBoundScriptListPane.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
