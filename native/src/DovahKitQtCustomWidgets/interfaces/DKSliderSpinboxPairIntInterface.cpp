#include "../widgets/DKSliderSpinboxPairInt.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKSliderSpinboxPairIntInterface.h"

DKSliderSpinboxPairIntInterface::DKSliderSpinboxPairIntInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKSliderSpinboxPairIntInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKSliderSpinboxPairIntInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKSliderSpinboxPairIntInterface::createWidget(QWidget* parent) {
   return new DKSliderSpinboxPairInt(parent);
}

QString DKSliderSpinboxPairIntInterface::name() const {
   return "DKSliderSpinboxPairInt";
}

QString DKSliderSpinboxPairIntInterface::group() const {
   return "DovahKit: Extended Qt Widgets";
}

QIcon DKSliderSpinboxPairIntInterface::icon() const {
   return QIcon(":/icons/slider.png");
}

QString DKSliderSpinboxPairIntInterface::toolTip() const {
   return tr("A paired slider and spinbox.");
}

QString DKSliderSpinboxPairIntInterface::whatsThis() const {
   return tr("A paired slider and spinbox.");
}

bool DKSliderSpinboxPairIntInterface::isContainer() const {
   return false;
}

QString DKSliderSpinboxPairIntInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKSliderSpinboxPairInt" name="slider">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKSliderSpinboxPairInt</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKSliderSpinboxPairIntInterface::includeFile() const {
   return "widgets/DKSliderSpinboxPairInt.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
