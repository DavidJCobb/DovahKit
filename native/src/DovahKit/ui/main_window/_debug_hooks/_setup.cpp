#include "_setup.h"
#include <QAction>
#include "../../../helpers/class_list.h"

#if _DEBUG
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
   #include "canvas_widget_tests.h"
   #include "load_nif.h"
   #include "papyrus_subsystem.h"
   #include "pex_parsing_benchmarks.h"
   #include "form_info_cache.h"
   #include "script_attachment_query_from_cache.h"
   #include "filter_object_selection_by_scriptname.h"
   #include "ui_form_list_pane_extra_col.h"
   #include "idles_datastore.h"
   #include "./form_types/imagespace_modifier_keyframe_interp.h"
   #include "./form_types/region_bifurcated_data_test.h"
   #include "./mega_tests/full_load_every_form.h"
   #include "./mega_tests/oops_all_itms.h"
   #include "./models/scoped_proxy_model.h"
   #include "./renderwin/vulkan_renderer_instance.h"
   #include "./renderwin/worldinput2.h"
   #include "./renderwin/worldinput_serialization.h"
   #include "./renderwin/worldinput2_input_seq_ui.h"
   #include "./renderwin/worldinput2_control_scheme_ui.h"
   #include "./renderwin/worldinput2_control_scheme_rebuild.h"
   #include "./qt/bulk_string_substitution.h"
   #include "./qt/paint_ellipse_tests.h"
   #include "./qt/qpalette.h"
   #include "./qt/qt_3d_tests.h"
   #include "./widgets/attack_data.h"
   #include "./widgets/audio_simple.h"
   #include "./widgets/bound_script_list_pane.h"
   #include "./widgets/breadcrumb_bar.h"
   #include "./widgets/ui_bsa_picker.h"
   #include "./widgets/collapsible_pane.h"
   #include "./widgets/float_slider.h"
   #include "./widgets/form_inventory.h"
   #include "./widgets/form_picker.h"
   #include "./widgets/key_picker.h"
   #include "./widgets/ref_picker.h"
   #include "./widgets/scene_editor.h"
   #include "./widgets/status_bar_segment.h"
   #include "./widgets/texture_asset_pane.h"
   #include "./widgets/yes_no_unset_widget.h"
#else
   #include "editor/ini/main.h"

   #include "./mega_tests/full_load_every_form.h"
   #include "./mega_tests/oops_all_itms.h"
#endif

namespace DovahKitDebug {
   #if _DEBUG
      using form_type_tests = cobb::class_list<
         features::form_types::imagespace_modifier_keyframe_interp,
         features::form_types::region_bifurcated_data_test
      >;
      using mega_tests = cobb::class_list<
         features::mega_tests::full_load_every_form,
         features::mega_tests::oops_all_itms
      >;
      using model_tests = cobb::class_list<
         features::models::scoped_proxy_model
      >;
      using renderwin_tests = cobb::class_list<
         features::renderwin::vulkan_renderer_instance,
         features::renderwin::worldinput2,
         features::renderwin::worldinput_serialization,
         features::renderwin::worldinput2_input_seq_ui,
         features::renderwin::worldinput2_control_scheme_ui,
         features::renderwin::worldinput2_control_scheme_rebuild
      >;
      using qt_tests = cobb::class_list <
         features::qt::bulk_string_substitution,
         features::qt::paint_ellipse_tests,
         features::qt::qt_3d_tests,
         features::qt::qpalette
      >;
      using widget_tests = cobb::class_list<
         features::widgets::attack_data,
         features::widgets::audio_simple,
         features::widgets::bound_script_list_pane,
         features::widgets::breadcrumb_bar,
         features::widgets::ui_bsa_picker,
         features::widgets::collapsible_pane,
         features::widgets::debug_form_picker,
         features::widgets::float_slider,
         features::widgets::form_inventory,
         features::widgets::key_picker,
         features::widgets::ref_picker,
         features::widgets::scene_editor,
         features::widgets::status_bar_segment,
         features::widgets::texture_asset_pane,
         features::widgets::yes_no_unset_widget
      >;

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
         features::debug_canvas_widget,
         features::load_nif,
         features::papyrus_subsystem,
         features::pex_parsing_benchmarks,
         features::form_info_cache,
         features::script_attachment_query_from_cache,
         features::filter_object_selection_by_scriptname,
         features::ui_form_list_pane_extra_col,
         features::idles_datastore
      >;
   #endif

   template<typename T>
   struct _add_functor {
      static void execute(QMenu* menu, QWidget* from) {
         auto* action = new QAction(menu);
         action->setText(T::name);
         QObject::connect(action, &QAction::triggered, [from]() { T::execute(from->window()); });
         menu->addAction(action);
      }
   };

   template<typename T>
   static void _add_menu(QWidget* parent, QMenu* menu, QString name) {
      if constexpr (T::count > 0) {
         auto* submenu = menu->addMenu(name);
         T::template for_each_with_args<_add_functor>(submenu, parent);
      }
   }

   extern void add_features_to_menu(QMenu* menu) {
      bool show_menu = false;
      #if !_DEBUG
         if (dovahkit::ini::main::debug::bShowMegaTests.get_current_value<bool>()) {
            auto* p = menu->parentWidget();
            if (p)
               p = p->window();
            _add_functor<features::mega_tests::full_load_every_form>::execute(menu, p);
            _add_functor<features::mega_tests::oops_all_itms>::execute(menu, p);

            show_menu = true;
         } else {
            show_menu = false;
         }
      #else
         show_menu = true;

         auto* p = menu->parentWidget();
         if (p)
            p = p->window();
         _add_menu<form_type_tests>(p, menu, QString("Form-type-specific tests"));
         _add_menu<renderwin_tests>(p, menu, QString("Render Window and friends"));
         _add_menu<qt_tests>(p, menu, QString("Qt"));
         _add_menu<widget_tests>(p, menu, QString("Widgets"));
         _add_menu<model_tests>(p, menu, QString("UI models"));
         _add_menu<mega_tests>(p, menu, QString("Mega-Tests"));
         all_features::for_each_with_args<_add_functor>(menu, p);
      #endif
         
      menu->setEnabled(show_menu);
      if (auto* action = menu->menuAction()) {
         action->setVisible(show_menu);
      }
   }
}