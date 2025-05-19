#include "../widgets/DKAttackDataWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKAttackDataWidgetInterface.h"

DKAttackDataWidgetInterface::DKAttackDataWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKAttackDataWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKAttackDataWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKAttackDataWidgetInterface::createWidget(QWidget* parent) {
   return new DKAttackDataWidget(parent);
}

QString DKAttackDataWidgetInterface::name() const {
   return "DKAttackDataWidget";
}

QString DKAttackDataWidgetInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKAttackDataWidgetInterface::icon() const {
   return QIcon(":/icons/DKAttackDataWidget.png");
}

QString DKAttackDataWidgetInterface::toolTip() const {
   return tr("A pane that can be used to edit attack data for an ActorBase or Race.");
}

QString DKAttackDataWidgetInterface::whatsThis() const {
   return QString();
}

bool DKAttackDataWidgetInterface::isContainer() const {
   return false;
}

QString DKAttackDataWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKAttackDataWidget" name="attackData">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKAttackDataWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKAttackDataWidgetInterface::includeFile() const {
   return "widgets/DKAttackDataWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
