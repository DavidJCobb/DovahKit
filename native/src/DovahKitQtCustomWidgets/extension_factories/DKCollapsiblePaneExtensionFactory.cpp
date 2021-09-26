#include "DKCollapsiblePaneExtensionFactory.h"

#include "../widgets/DKCollapsiblePane.h"
#include "../extensions/DKCollapsiblePaneContainerExtension.h"

DKCollapsiblePaneExtensionFactory::DKCollapsiblePaneExtensionFactory(QExtensionManager* parent) : QExtensionFactory(parent) {}

QObject* DKCollapsiblePaneExtensionFactory::createExtension(QObject* object, const QString& iid, QObject* parent) const {
   auto* widget = qobject_cast<DKCollapsiblePane*>(object);
   if (widget && (iid == Q_TYPEID(QDesignerContainerExtension)))
      return new DKCollapsiblePaneContainerExtension(widget, parent);
   return nullptr;
}