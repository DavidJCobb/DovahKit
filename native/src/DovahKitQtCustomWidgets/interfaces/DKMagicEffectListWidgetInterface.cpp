#include "../widgets/DKMagicEffectListWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKMagicEffectListWidgetInterface.h"

DKMagicEffectListWidgetInterface::DKMagicEffectListWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKMagicEffectListWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKMagicEffectListWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKMagicEffectListWidgetInterface::createWidget(QWidget* parent) {
   return new DKMagicEffectListWidget(parent);
}

QString DKMagicEffectListWidgetInterface::name() const {
   return "DKMagicEffectListWidget";
}

QString DKMagicEffectListWidgetInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKMagicEffectListWidgetInterface::icon() const {
   return QIcon(":/icons/listpane.png");
}

QString DKMagicEffectListWidgetInterface::toolTip() const {
   return tr("An editor for Magic Effect lists on a form.");
}

QString DKMagicEffectListWidgetInterface::whatsThis() const {
   return tr("An editor for Magic Effect lists on a form.");
}

bool DKMagicEffectListWidgetInterface::isContainer() const {
   return false;
}

QString DKMagicEffectListWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKMagicEffectListWidget" name="effects">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKMagicEffectListWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKMagicEffectListWidgetInterface::includeFile() const {
   return "widgets/DKMagicEffectListWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
