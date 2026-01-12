#pragma once
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <QAbstractItemModel>
#include "helpers/type_traits/is_std_vector.h"
#include "helpers/vectors/re_sort_item_within.h"

namespace ui::model_utils {
   namespace impl::self_sorting_flat_model_mixin {
      template<typename T>
      concept has_data_vector = requires(T& model) {
         requires cobb::is_std_vector<decltype(T::_data)>;
         { model._data };
      };
      
      template<typename T>
      concept has_sort_comparator = requires(const T& cm) {
         { T::_sort_comparator(cm._data[0], cm._data[0]) } -> std::same_as<bool>;
      };

      template<typename T>
      concept is_valid = requires(T & model, const T & const_model) {
         requires std::is_base_of_v<QAbstractItemModel, T>;
         requires has_data_vector<T>;
         requires has_sort_comparator<T>;
      };
   }

   //
   // Uses CRTP.
   //
   template<typename Self>
   class self_sorting_flat_model_mixin {
      private:
         QAbstractItemModel* model() requires impl::self_sorting_flat_model_mixin::is_valid<Self> {
            return static_cast<Self*>(this);
         }
         auto& rows() requires impl::self_sorting_flat_model_mixin::is_valid<Self> {
            return static_cast<Self*>(this)->_data;
         }

      public:
         auto _insertion_point_for(const auto& item) requires impl::self_sorting_flat_model_mixin::is_valid<Self> {
            auto& list = this->rows();
            return std::upper_bound(
               list.begin(),
               list.end(),
               item,
               &Self::_sort_comparator
            );
         }
         void _do_sorted_insertion(auto&& item) requires impl::self_sorting_flat_model_mixin::is_valid<Self> {
            auto& list  = this->rows();
            auto* model = this->model();

            auto   it = _insertion_point_for(item);
            size_t i  = std::distance(list.begin(), it);
            model->beginInsertRows({}, i, i);
            list.insert(it, std::move(item));
            model->endInsertRows();
         }
         void _re_sort_item(size_t from) requires impl::self_sorting_flat_model_mixin::is_valid<Self> {
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