#pragma once
#include <functional>
#include <type_traits>
#include <QWidget>
#include "../_ui_base.h"

#include "../../constants/ui_count_limits.h"

class DovahscriptDialog;

namespace dovahscript::tasks::s2m {
   namespace impl::create_ui_widget {
      extern int  get_window_count();
      extern void set_up_widget(QWidget&);
   }

   template<typename widget_type> requires std::is_base_of_v<QWidget, widget_type>
   class create_ui_widget : public _ui_write_base {
      public:
         create_ui_widget() {}

         std::function<void(widget_type*)> configure;
         widget_type* created = nullptr;
         
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override {
            if constexpr (std::is_same_v<DovahscriptDialog, widget_type>) {
               if (impl::create_ui_widget::get_window_count() >= max_script_windows) {
                  return;
               }
            }
            created = new widget_type;
            impl::create_ui_widget::set_up_widget(*created);
            if (this->configure)
               this->configure(created);
         }
   };
}