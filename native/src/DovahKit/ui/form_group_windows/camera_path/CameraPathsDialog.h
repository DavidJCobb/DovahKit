#pragma once
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_camera_path.h" // generated
#include "dovah/forms/CameraPath.h"
#include "dovah/form_stub.h"
namespace dovah::exceptions {
   class form_creation_failed;
}
class CameraPathFormsModel;

class CameraPathsDialog : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::CameraPath;

   public:
      CameraPathsDialog(QWidget* parent = nullptr);

      void focusCameraPath(dovah::form_stub&);

      virtual bool eventFilter(QObject* watched, QEvent*) override;

   protected:
      Ui::CameraPathsDialog ui;
      struct {
         QMenu* menu = nullptr;
         struct {
            QAction* create_sibling = nullptr;
            QAction* create_child   = nullptr;
            QMenu*   duplicate      = nullptr;
            QAction* del            = nullptr;
            QAction* use_info       = nullptr;
         } actions;
      } _context;
      dovah::loaded_form_ptr<loaded_form_type> _current_form;
      CameraPathFormsModel* _model = nullptr;

      QModelIndex _get_selected_row();

      #pragma region Idle tree context menu
         void _context_create_sibling();
         void _context_create_child();
         void _context_duplicate_single();
         void _context_duplicate_tree();
         void _context_delete_idle();
         void _context_to_canonical();
         void _context_use_info();
      #pragma endregion
         
      void _keybind_delete_idle();

      void _report_idle_create_error(const dovah::exceptions::form_creation_failed&);

      void _update_move_button_enable_states();

      void _pull_selected_idle_to_ui();
      void _push_selected_idle_to_form();
      void _set_form_ui_enable_state(bool);
};