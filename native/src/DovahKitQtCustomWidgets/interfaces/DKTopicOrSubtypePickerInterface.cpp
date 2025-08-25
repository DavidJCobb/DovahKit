#include "./DKTopicOrSubtypePickerInterface.h"
#include "../widgets/DKTopicOrSubtypePicker.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

DKTopicOrSubtypePickerInterface::DKTopicOrSubtypePickerInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKTopicOrSubtypePickerInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKTopicOrSubtypePickerInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKTopicOrSubtypePickerInterface::createWidget(QWidget* parent) {
   return new DKTopicOrSubtypePicker(parent);
}

QString DKTopicOrSubtypePickerInterface::name() const {
   return "DKTopicOrSubtypePicker";
}

QString DKTopicOrSubtypePickerInterface::group() const {
   return "DovahKit: Forms and form data";
}

QIcon DKTopicOrSubtypePickerInterface::icon() const {
   return QIcon();
}

QString DKTopicOrSubtypePickerInterface::toolTip() const {
   return tr("A drop-down that can be used to select a topic subtype or a topic form.");
}

QString DKTopicOrSubtypePickerInterface::whatsThis() const {
   return QString();
}

bool DKTopicOrSubtypePickerInterface::isContainer() const {
   return false;
}

QString DKTopicOrSubtypePickerInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKTopicOrSubtypePicker" name="topicOrSubtype">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKTopicOrSubtypePicker</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKTopicOrSubtypePickerInterface::includeFile() const {
   return "widgets/DKTopicOrSubtypePicker.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
