#include "../widgets/DKSliderSpinboxPairFloat.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKSliderSpinboxPairFloatInterface.h"

DKSliderSpinboxPairFloatInterface::DKSliderSpinboxPairFloatInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKSliderSpinboxPairFloatInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKSliderSpinboxPairFloatInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKSliderSpinboxPairFloatInterface::createWidget(QWidget* parent) {
   return new DKSliderSpinboxPairFloat(parent);
}

QString DKSliderSpinboxPairFloatInterface::name() const {
   return "DKSliderSpinboxPairFloat";
}

QString DKSliderSpinboxPairFloatInterface::group() const {
   return "DovahKit: Extended Qt Widgets";
}

QIcon DKSliderSpinboxPairFloatInterface::icon() const {
   return QIcon(":/icons/slider.png");
}

QString DKSliderSpinboxPairFloatInterface::toolTip() const {
   return tr("A paired slider and spinbox.");
}

QString DKSliderSpinboxPairFloatInterface::whatsThis() const {
   return tr("A paired slider and spinbox.");
}

bool DKSliderSpinboxPairFloatInterface::isContainer() const {
   return false;
}

QString DKSliderSpinboxPairFloatInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKSliderSpinboxPairFloat" name="slider">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKSliderSpinboxPairFloat</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKSliderSpinboxPairFloatInterface::includeFile() const {
   return "widgets/DKSliderSpinboxPairFloat.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
