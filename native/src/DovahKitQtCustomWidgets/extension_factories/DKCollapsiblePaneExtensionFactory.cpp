#include "DKCollapsiblePaneExtensionFactory.h"

#include "../widgets/DKCollapsiblePane.h"
#include "../extensions/DKCollapsiblePaneContainerExtension.h"
#include "../extensions/DKCollapsiblePaneTaskMenuExtension.h"

DKCollapsiblePaneExtensionFactory::DKCollapsiblePaneExtensionFactory(QExtensionManager* parent) : QExtensionFactory(parent) {}

QObject* DKCollapsiblePaneExtensionFactory::createExtension(QObject* object, const QString& iid, QObject* parent) const {
   auto* widget = qobject_cast<DKCollapsiblePane*>(object);
   if (!widget)
      return nullptr;
   if (iid == Q_TYPEID(QDesignerContainerExtension))
      return new DKCollapsiblePaneContainerExtension(widget, parent);
   if (iid == Q_TYPEID(QDesignerTaskMenuExtension))
      return new DKCollapsiblePaneTaskMenuExtension(widget, parent);
   return nullptr;
}