#include "DKYesNoUnsetWidgetExtensionFactory.h"

#include "../widgets/DKYesNoUnsetWidget.h"
#include "../extensions/DKYesNoUnsetWidgetTaskMenuExtension.h"

DKYesNoUnsetWidgetExtensionFactory::DKYesNoUnsetWidgetExtensionFactory(QExtensionManager* parent) : QExtensionFactory(parent) {}

QObject* DKYesNoUnsetWidgetExtensionFactory::createExtension(QObject* object, const QString& iid, QObject* parent) const {
   auto* widget = qobject_cast<DKYesNoUnsetWidget*>(object);
   if (!widget)
      return nullptr;
   if (iid == Q_TYPEID(QDesignerTaskMenuExtension))
      return new DKYesNoUnsetWidgetTaskMenuExtension(widget, parent);
   return nullptr;
}