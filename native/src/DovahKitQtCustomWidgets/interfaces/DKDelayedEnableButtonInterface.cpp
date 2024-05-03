#include "../widgets/DKDelayedEnableButton.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKDelayedEnableButtonInterface.h"

DKDelayedEnableButtonInterface::DKDelayedEnableButtonInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKDelayedEnableButtonInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   qRegisterMetaType<DKDelayedEnableButton_SpinnerStyleOptions>();
   this->initialized = true;
}

bool DKDelayedEnableButtonInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKDelayedEnableButtonInterface::createWidget(QWidget* parent) {
   return new DKDelayedEnableButton(parent);
}

QString DKDelayedEnableButtonInterface::name() const {
   return "DKDelayedEnableButton";
}

QString DKDelayedEnableButtonInterface::group() const {
   return "DovahKit: General";
}

QIcon DKDelayedEnableButtonInterface::icon() const {
   return QIcon();
}

QString DKDelayedEnableButtonInterface::toolTip() const {
   return tr("A button that can be set to enable itself only after a delay, with the delay displayed to the user.");
}

QString DKDelayedEnableButtonInterface::whatsThis() const {
   return tr("A button that can be set to enable itself only after a delay, with the delay displayed to the user.");
}

bool DKDelayedEnableButtonInterface::isContainer() const {
   return false;
}

QString DKDelayedEnableButtonInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKDelayedEnableButton" name="button">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKDelayedEnableButton</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKDelayedEnableButtonInterface::includeFile() const {
   return "widgets/DKDelayedEnableButton.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
