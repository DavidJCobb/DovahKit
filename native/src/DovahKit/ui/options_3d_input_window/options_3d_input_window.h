#pragma once
#include "ui_options_3d_input_window.h"
#include "widgets/DKBoundInputWidget.h"

namespace DK3D {
   struct Binding;
}
namespace DK3DToolOptions {
   class Base;
}

class Options3DInputDialog : public QDialog {
   Q_OBJECT;
   protected:
      Options3DInputDialog(QWidget* parent = nullptr);
      static Options3DInputDialog* current_instance;

   public:
      static Options3DInputDialog* open(QWidget* parent = nullptr);
      ~Options3DInputDialog();

   protected:
      struct {
         DKBoundInputWidget*    input        = nullptr;
         DK3DToolOptions::Base* tool_options = nullptr;
      } subwidgets;
      struct {
         QVector<DK3D::Binding> bindings;
         int selected_binding = -1;
      } state;

   public slots:
      void setBindings(QVector<DK3D::Binding>);

   protected:
      void _updateBindList();
      DK3D::Binding* selectedBinding();

   protected slots:
      void bindingSelected();
      void rebuildToolOptions();

   private:
      Ui::Options3DInputDialog ui;
};