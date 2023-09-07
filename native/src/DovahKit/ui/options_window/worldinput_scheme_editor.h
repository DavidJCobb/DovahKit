#pragma once
#include <QDialog>
#include "ui_worldinput_scheme_editor.h"

#include "editor/subsystems/worldinput2/enums/input_device_type.h"

namespace dovahkit::subsystems::worldinput2 {
   class control_scheme;
}
class DKWorldinputControlSchemeModel;

class WorldinputSchemeEditDialog : public QDialog {
   Q_OBJECT;
   public:
      using input_device_type = dovahkit::subsystems::worldinput2::input_device_type;
      using control_scheme_type = dovahkit::subsystems::worldinput2::control_scheme;
      
   public:
      WorldinputSchemeEditDialog(input_device_type, QWidget* parent = nullptr);

      void initializeFrom(const control_scheme_type&);
      void overwrite(control_scheme_type&) const;
      control_scheme_type retrieve() const;

   protected:
      Ui::WorldinputSchemeEditDialog ui;
      //
      const input_device_type device_type;
      DKWorldinputControlSchemeModel* _model = nullptr;

      QModelIndex _getFirstSelectedNode();
      const QItemSelection _getSelection();
};