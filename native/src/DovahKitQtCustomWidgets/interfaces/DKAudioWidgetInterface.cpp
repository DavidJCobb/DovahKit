#include "../widgets/DKAudioWidget.h"
#include <QtCore/QtPlugin>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include "DKAudioWidgetInterface.h"

DKAudioWidgetInterface::DKAudioWidgetInterface(QObject* parent) : QObject(parent) {
   this->initialized = false;
}

void DKAudioWidgetInterface::initialize(QDesignerFormEditorInterface* intfc) {
   if (this->initialized)
      return;
   //
   QExtensionManager* manager = intfc->extensionManager();
   assert(manager);
   //
   this->initialized = true;
}

bool DKAudioWidgetInterface::isInitialized() const {
   return this->initialized;
}

QWidget* DKAudioWidgetInterface::createWidget(QWidget* parent) {
   return new DKAudioWidget(parent);
}

QString DKAudioWidgetInterface::name() const {
   return "DKAudioWidget";
}

QString DKAudioWidgetInterface::group() const {
   return "DovahKit: General";
}

QIcon DKAudioWidgetInterface::icon() const {
   return QIcon();
}

QString DKAudioWidgetInterface::toolTip() const {
   return tr("A widget that can be used to play, pause, stop, or seek an audio file.");
}

QString DKAudioWidgetInterface::whatsThis() const {
   return tr("This widget can load an audio file and play, pause, or stop it. It also supports a seek slider. The widget is inseparable from DovahKit's audio subsystem.");
}

bool DKAudioWidgetInterface::isContainer() const {
   return false;
}

QString DKAudioWidgetInterface::domXml() const {
   return R"555(
<ui language="c++">
   <widget class="DKAudioWidget" name="aliasPicker">
   </widget>
   <customwidgets>
      <customwidget>
         <class>DKAudioWidget</class>
         <extends>QWidget</extends>
      </customwidget>
   </customwidgets>
</ui>
)555";
}

QString DKAudioWidgetInterface::includeFile() const {
   return "widgets/DKAudioWidget.h"; // NOTE for MSVC: this will require that your project specify $(ProjectDir) as an include path, else UI files in subfolders will resolve this incorrectly
}
