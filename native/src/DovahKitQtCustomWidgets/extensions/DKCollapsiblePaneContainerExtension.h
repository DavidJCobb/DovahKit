#pragma once
#include <QtDesigner/QDesignerContainerExtension>
#include "../widgets/DKCollapsiblePane.h"

class QExtensionManager;

class DKCollapsiblePane;

// The only way to allow Qt Designer to cleanly work with "viewport"-style widgets like 
// QScrollArea is to define them as "multi-page" widgets that just stubbornly refuse to 
// ever have more than one page.
class DKCollapsiblePaneContainerExtension : public QObject, public QDesignerContainerExtension {
   Q_OBJECT;
   Q_INTERFACES(QDesignerContainerExtension);
   public:
      explicit DKCollapsiblePaneContainerExtension(DKCollapsiblePane* widget, QObject* parent) : QObject(parent), instance(widget) {};

      virtual void addWidget(QWidget* widget) override {
         assert(instance->viewport() == nullptr);
         instance->setViewport(widget);
      }

      virtual bool canAddWidget() const override { return instance->viewport() == nullptr; }

      virtual int count() const override { return instance->viewport() ? 1 : 0; }

      virtual int currentIndex() const override { return instance->viewport() ? 0 : -1; }

      virtual void insertWidget(int index, QWidget* widget) override { this->addWidget(widget); }

      virtual void remove(int index) override {}

      virtual void setCurrentIndex(int index) override {}

      virtual QWidget* widget(int index) const override { return instance->viewport(); }

   private:
      DKCollapsiblePane* instance = nullptr;
};