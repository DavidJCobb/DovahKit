#pragma once
#include <algorithm>
#include <QAbstractItemModel>
#include "helpers/vectors/re_sort_item_within.h"

namespace ui::model_utils {
   //
   // Uses CRTP.
   //
   template<typename Self>
   class self_sorting_flat_model_mixin {
      private:
         Self* model() {
            return static_cast<Self*>(this);
         }
         auto& rows() {
            return static_cast<Self*>(this)->_data;
         }

      public:
         auto _insertion_point_for(const auto& item) {
            auto& list = this->rows();
            return std::upper_bound(
               list.begin(),
               list.end(),
               item,
               &Self::_sort_comparator
            );
         }
         void _do_sorted_insertion(auto&& item) {
            auto& list  = this->rows();
            auto* model = this->model();

            auto   it = _insertion_point_for(item);
            size_t i  = std::distance(list.begin(), it);
            model->beginInsertRows({}, i, i);
            list.insert(it, std::move(item));
            model->endInsertRows();
         }
         void _re_sort_item(size_t from) {
            auto& list = this->rows();
            if (from >= list.size())
               return;
            bool  moved = false;
            auto* model = this->model();
            cobb::vectors::re_sort_item_within(
               list,
               list.begin() + from,
               &Self::_sort_comparator,
               [&moved, model, &list](auto from_it, auto to_it) {
                  moved = true;
                  size_t from  = std::distance(list.begin(), from_it);
                  size_t to    = std::distance(list.begin(), to_it);
                  model->beginMoveRows(
                     {},
                     from, // first to move
                     from, // last  to move
                     {},
                     (to < from) ? to : to + 1 // Qt API design jank
                  );
               }
            );
            if (moved)
               model->endMoveRows();
         }
   };
}