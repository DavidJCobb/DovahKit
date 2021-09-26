#pragma once
#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class DKCollapsiblePaneExtensionFactory : public QExtensionFactory {
   Q_OBJECT;
   public:
      explicit DKCollapsiblePaneExtensionFactory(QExtensionManager* parent = nullptr);

   protected:
      QObject* createExtension(QObject* object, const QString& iid, QObject* parent) const override;
};