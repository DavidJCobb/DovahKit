#pragma once
#include <QSpinBox>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/set_edit_gizmo_mode.h"
#include "ui_set_edit_gizmo_mode_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class set_edit_gizmo_mode : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::set_edit_gizmo_mode;
         using options_type = tool::options;

      public:
         set_edit_gizmo_mode(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsSetEditGizmoMode ui;
         struct {
            options_type current_options = {};
         } _state;
         struct {
            struct {
               EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> a;
               EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> b;
            } frames;
            struct {
               EnumCombobox<dovahkit::subsystems::worldedit::gizmo_mode> a;
               EnumCombobox<dovahkit::subsystems::worldedit::gizmo_mode> b;
            } modes;
         } _widget_wrappers;
   };
}