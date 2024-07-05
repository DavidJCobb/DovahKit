#include "../widgets/DKTextureAssetPane.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKTextureAssetPaneInterface.h"

DKTextureAssetPaneInterface::DKTextureAssetPaneInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKTextureAssetPaneInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKTextureAssetPaneInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKTextureAssetPaneInterface::createWidget(QWidget* parent) {
   return new DKTextureAssetPane(parent);
}

QString DKTextureAssetPaneInterface::name() const {
   return "DKTextureAssetPane";
}

QString DKTextureAssetPaneInterface::group() const {
   return "DovahKit: Game asset selection";
}

QIcon DKTextureAssetPaneInterface::icon() const {
   return QIcon();
}

QString DKTextureAssetPaneInterface::toolTip() const {
   return tr("A pane that can show a DDS texture asset selected from the game files.");
}

QString DKTextureAssetPaneInterface::whatsThis() const {
   return QString();
}

bool DKTextureAssetPaneInterface::isContainer() const {
   return false;
}

QString DKTextureAssetPaneInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKTextureAssetPane" name="textureAssetPane">
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
         <class>DKTextureAssetPane</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKTextureAssetPaneInterface::includeFile() const {
   return "widgets/DKTextureAssetPane.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
