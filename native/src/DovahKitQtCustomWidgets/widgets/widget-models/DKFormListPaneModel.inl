#include "./DKFormListPaneModel.h"

/*static*/ constexpr std::optional<size_t> DKFormListPaneModel::Item::cache_index_for_builtin_column(Column::Enum col) {
   switch (col) {
      case Column::Type: return 0;
      case Column::Name: return 1;
   }
   return {};
}
/*static*/ constexpr size_t DKFormListPaneModel::Item::cache_index_for_extra_column(size_t extra_col_index) {
   return cached_builtin_column_count + extra_col_index;
}