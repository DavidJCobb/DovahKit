#pragma once
#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class DKYesNoUnsetWidgetExtensionFactory : public QExtensionFactory {
   Q_OBJECT;
   public:
      explicit DKYesNoUnsetWidgetExtensionFactory(QExtensionManager* parent = nullptr);

   protected:
      QObject* createExtension(QObject* object, const QString& iid, QObject* parent) const override;
};