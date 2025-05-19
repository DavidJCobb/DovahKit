#include "../widgets/DKFloatSlider.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKFloatSliderInterface.h"

DKFloatSliderInterface::DKFloatSliderInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKFloatSliderInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKFloatSliderInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKFloatSliderInterface::createWidget(QWidget* parent) {
   return new DKFloatSlider(parent);
}

QString DKFloatSliderInterface::name() const {
   return "DKFloatSlider";
}

QString DKFloatSliderInterface::group() const {
   return "DovahKit: Extended Qt Widgets";
}

QIcon DKFloatSliderInterface::icon() const {
   return QIcon(":/icons/slider.png");
}

QString DKFloatSliderInterface::toolTip() const {
   return tr("A slider control that operates with floating-point values instead of integers.");
}

QString DKFloatSliderInterface::whatsThis() const {
   return tr("A slider control that operates with floating-point values instead of integers. Under the hood, it just wraps a QSlider and uses a multiplier.");
}

bool DKFloatSliderInterface::isContainer() const {
   return false;
}

QString DKFloatSliderInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKFloatSlider" name="slider">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKFloatSlider</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKFloatSliderInterface::includeFile() const {
   return "widgets/DKFloatSlider.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
