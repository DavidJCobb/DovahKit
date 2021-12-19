#pragma once
#include "ui_options_3d_input_window.h"
#include "widgets/DKBoundInputWidget.h"

class Options3DControlSchemeModel;
namespace DK3D::binds {
   class tree;
   namespace nodes {
      class input;
   }
}
namespace DK3DToolOptions {
   class Base;
}

class Options3DInputDialog : public QDialog {
   Q_OBJECT;
   protected:
      Options3DInputDialog(QWidget* parent = nullptr);
      static Options3DInputDialog* current_instance;

      using model_type = Options3DControlSchemeModel;

   public:
      static Options3DInputDialog* open(QWidget* parent = nullptr);
      ~Options3DInputDialog();

   protected:
      struct {
         DKBoundInputWidget*    input        = nullptr;
         DK3DToolOptions::Base* tool_options = nullptr;
      } subwidgets;
      struct {
         model_type* model = nullptr;
      } state;

      DK3D::binds::nodes::input* selectedInputNode() const;

   public slots:
      void setBindings(const DK3D::binds::tree&);

   protected:
      void _updateBindListButtons(const QModelIndex& target = QModelIndex());

   protected slots:
      void bindingSelected();
      void rebuildToolOptions(DK3D::binds::nodes::input* node = nullptr);

      void addBind();
      void deleteBind();
      void moveBind(int);
      void moveBindUp();
      void moveBindDown();

   private:
      Ui::Options3DInputDialog ui;
};