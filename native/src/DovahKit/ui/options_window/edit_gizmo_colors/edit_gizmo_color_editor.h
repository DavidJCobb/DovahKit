#pragma once
#include <QDialog>
#include "ui_edit_gizmo_color_editor.h"

#include "editor/subsystems/worldedit/gizmo_colors/gizmo_color_scheme.h"

class EditGizmoColorSchemeEditDialog : public QDialog {
   Q_OBJECT;
   public:
      using gizmo_color_scheme = dovahkit::subsystems::worldedit::gizmo_color_scheme;
      
   public:
      EditGizmoColorSchemeEditDialog(QWidget* parent = nullptr);

      void initializeFrom(const gizmo_color_scheme&);
      void overwrite(gizmo_color_scheme&) const;

   protected:
      Ui::EditGizmoColorSchemeEditDialog ui;
};