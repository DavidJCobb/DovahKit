#pragma once
#include <QComboBox>
#include "dovah/data/actor_values.h"

namespace ui::enum_dropdown_configs {
   namespace impl {
      extern QString actor_value_form_name(const dovah::actor_value_info& av_info);

      inline bool default_actor_value_filter(const dovah::actor_value_info& av_info) {
         return true;
      }
   }

   struct actor_value_index_options {
      bool allow_none     = true;
      bool sorted         = false;
      bool use_form_names = false;
   };

   template<actor_value_index_options Options, typename FilterFunction>
   extern void actor_value_index(QComboBox* widget, FilterFunction&& filter) {
      widget->clear();
      for (auto& av_info : dovah::all_actor_value_info) {
         if (!filter(av_info))
            continue;
         QString name;
         if constexpr (Options.use_form_names) {
            name = impl::actor_value_form_name(av_info);
            if (name.isEmpty())
               name = QString::fromLatin1(av_info.name.data(), av_info.name.size());
         } else {
            name = QString::fromLatin1(av_info.name.data(), av_info.name.size());
         }
         widget->addItem(name, (int)av_info.index);
      }
      if constexpr (Options.sorted) {
         widget->model()->sort(0);
      }
      if constexpr (Options.allow_none) {
         widget->insertItem(0, QObject::tr("NONE", "no actor value"), -1);
      }
   }
   
   template<actor_value_index_options Options>
   extern void actor_value_index(QComboBox* widget) {
      actor_value_index<Options>(widget, &impl::default_actor_value_filter);
   }
}