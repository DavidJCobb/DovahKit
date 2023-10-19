#pragma once
#include <QButtonGroup>
#include <QWidget>
#include "helpers/qt/wrappers/enum_combobox.h"

#include "./_base.h"

#include "editor/subsystems/worldedit/tool_system/tools/move_selection_by_drag.h"
#include "ui_move_selection_by_drag_tool_options.h" // generated

namespace dovahkit::ui::worldedit::tools {
   class move_selection_by_drag : public base {
      public:
         using tool         = dovahkit::subsystems::worldedit::tools::move_selection_by_drag;
         using options_type = tool::options;

      public:
         move_selection_by_drag(QWidget* parent = nullptr);

         const options_type& get_options() const { return this->_state.current_options; }
         void set_options(const options_type&);

      protected:
         template<typename T> using EnumCombobox = cobb::qt::wrappers::EnumCombobox<T>;

         Ui::WorldeditToolOptionsMoveSelectionByDrag ui;
         struct {
            options_type current_options = {};
         } _state;
         QButtonGroup* _typeButtonGroup = nullptr;
         struct {
            EnumCombobox<dovahkit::subsystems::worldedit::reference_frame> frame;
            EnumCombobox<dovahkit::subsystems::worldedit::axis3D> drag_axis_axis;
            EnumCombobox<dovahkit::subsystems::worldedit::axis3D> drag_plane_normal;
         } _widget_wrappers;
   };
}