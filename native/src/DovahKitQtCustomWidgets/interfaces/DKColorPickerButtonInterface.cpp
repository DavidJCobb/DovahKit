#include "../widgets/DKColorPickerButton.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKColorPickerButtonInterface.h"

DKColorPickerButtonInterface::DKColorPickerButtonInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKColorPickerButtonInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKColorPickerButtonInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKColorPickerButtonInterface::createWidget(QWidget* parent) {
   return new DKColorPickerButton(parent);
}

QString DKColorPickerButtonInterface::name() const {
   return "DKColorPickerButton";
}

QString DKColorPickerButtonInterface::group() const {
   return "DovahKit";
}

QIcon DKColorPickerButtonInterface::icon() const {
   return QIcon();
}

QString DKColorPickerButtonInterface::toolTip() const {
   return tr("A button that shows a color; clicking it opens a QColorDialog.");
}

QString DKColorPickerButtonInterface::whatsThis() const {
   return QString();
}

bool DKColorPickerButtonInterface::isContainer() const {
   return false;
}

QString DKColorPickerButtonInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKColorPickerButton" name="colorButton">
      <property name="geometry">
         <rect>
            <x>0</x>
            <y>0</y>
            <width>100</width>
            <height>32</height>
         </rect>
      </property>
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKColorPickerButton</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKColorPickerButtonInterface::includeFile() const {
   return "widgets/DKColorPickerButton.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
