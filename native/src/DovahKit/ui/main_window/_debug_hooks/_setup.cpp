#include "_setup.h"
#include <QAction>
#include "../../../helpers/class_list.h"

#include "get_record_size_stats.h"
#include "list_none_stubs.h"
#include "compiled_papyrus_script_tests.h"
#include "debug_target_form.h"
#include "debug_target_form_papyrus.h"
#include "extract_bsa_file.h"
#include "enumerate_bsa_contents.h"
#include "lookup_bsa_file_from_bsa_load_order.h"
#include "lua_resource_manager_tests.h"
#include "qt_ini_tests.h"
#include "form_picker_debug.h"
#include "canvas_widget_tests.h"
#include "ui_collapsible_pane.h"
#include "qt_paint_ellipse_tests.h"
#include "ui_bsa_picker.h"
#include "ui_texture_asset_pane.h"
#include "qt_3d_tests.h"
#include "vulkan_renderer_instance.h"
#include "ui_key_picker.h"
#include "load_nif.h"
#include "worldinput2.h"
#include "worldinput_serialization.h"
#include "worldinput2_input_seq_ui.h"
#include "worldinput2_control_scheme_ui.h"
#include "worldinput2_control_scheme_rebuild.h"
#include "ui_ref_picker.h"
#include "papyrus_subsystem.h"
#include "pex_parsing_benchmarks.h"
#include "form_info_cache.h"
#include "script_attachment_query_from_cache.h"
#include "filter_object_selection_by_scriptname.h"
#include "ui_bound_script.h"
#include "ui_form_inventory.h"
#include "ui_attack_data.h"
#include "ui_form_list_pane_extra_col.h"

namespace DovahKitDebug {
   using all_features = cobb::class_list<
      features::get_record_size_stats,
      features::list_none_stubs,
      features::compiled_papyrus_script_tests,
      features::debug_target_form,
      features::debug_target_form_papyrus,
      features::extract_bsa_file,
      features::enumerate_bsa_contents,
      features::lookup_bsa_file_from_bsa_load_order,
      features::run_lua_resource_manager_tests,
      features::qt_ini_tests,
      features::debug_form_picker,
      features::debug_canvas_widget,
      features::ui_collapsible_pane,
      features::qt_paint_ellipse_tests,
      features::ui_bsa_picker,
      features::ui_texture_asset_pane,
      features::qt_3d_tests,
      features::vulkan_renderer_instance,
      features::ui_key_picker,
      features::load_nif,
      features::worldinput2,
      features::worldinput_serialization,
      features::worldinput2_input_seq_ui,
      features::worldinput2_control_scheme_ui,
      features::worldinput2_control_scheme_rebuild,
      features::ui_ref_picker,
      features::papyrus_subsystem,
      features::pex_parsing_benchmarks,
      features::form_info_cache,
      features::script_attachment_query_from_cache,
      features::filter_object_selection_by_scriptname,
      features::ui_bound_script,
      features::ui_form_inventory,
      features::ui_attack_data,
      features::ui_form_list_pane_extra_col//,
   >;

   template<typename T> struct _add_functor {
      static void execute(QMenu* menu, QWidget* from) {
         auto* action = new QAction(menu);
         action->setText(T::name);
         QObject::connect(action, &QAction::triggered, [from]() { T::execute(from->window()); });
         menu->addAction(action);
      }
   };

   extern void add_features_to_menu(QMenu* menu) {
      #if !_DEBUG
         menu->setVisible(false);
         menu->setEnabled(false);
         return;
      #endif
      menu->setVisible(true);
      menu->setEnabled(true);
      auto* p = menu->parentWidget();
      if (p)
         p = p->window();
      all_features::for_each_with_args<_add_functor>(menu, p);
   }
}