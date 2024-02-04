#include "../widgets/DKQuestAliasPicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKQuestAliasPickerInterface.h"

DKQuestAliasPickerInterface::DKQuestAliasPickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKQuestAliasPickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKQuestAliasPickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKQuestAliasPickerInterface::createWidget(QWidget* parent) {
   return new DKQuestAliasPicker(parent);
}

QString DKQuestAliasPickerInterface::name() const {
   return "DKQuestAliasPicker";
}

QString DKQuestAliasPickerInterface::group() const {
   return "DovahKit";
}

QIcon DKQuestAliasPickerInterface::icon() const {
   return QIcon();
}

QString DKQuestAliasPickerInterface::toolTip() const {
   return tr("A widget that can be used to select a quest alias.");
}

QString DKQuestAliasPickerInterface::whatsThis() const {
   return tr("This widget consists of drop-down menus that can be used to select a quest and an alias in that quest. Aliases can optionally be limited to specific types.");
}

bool DKQuestAliasPickerInterface::isContainer() const {
   return false;
}

QString DKQuestAliasPickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKQuestAliasPicker" name="aliasPicker">
      <property name="geometry">
         <rect>
            <x>0</x>
            <y>0</y>
            <width>100</width>
         </rect>
      </property>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKQuestAliasPicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKQuestAliasPickerInterface::includeFile() const {
   return "widgets/DKQuestAliasPicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
