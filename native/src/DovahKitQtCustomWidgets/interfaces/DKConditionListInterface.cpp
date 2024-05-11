#include "../widgets/DKConditionList.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKConditionListInterface.h"

DKConditionListInterface::DKConditionListInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKConditionListInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKConditionListInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKConditionListInterface::createWidget(QWidget* parent) {
   return new DKConditionList(parent);
}

QString DKConditionListInterface::name() const {
   return "DKConditionList";
}

QString DKConditionListInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKConditionListInterface::icon() const {
   return QIcon();
}

QString DKConditionListInterface::toolTip() const {
   return tr("An editor for conditions on a form.");
}

QString DKConditionListInterface::whatsThis() const {
   return tr("An editor for conditions on a form: a listview and buttons for modification.");
}

bool DKConditionListInterface::isContainer() const {
   return false;
}

QString DKConditionListInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKConditionList" name="conditions">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKConditionList</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKConditionListInterface::includeFile() const {
   return "widgets/DKConditionList.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
