#pragma once
#include "./DKBoundScriptModel.h"

constexpr bool DKBoundScriptModel::anyPropertiesEditedLocally() const {
   return this->_cached.any_properties_edited_locally;
}
constexpr bool DKBoundScriptModel::anyPropertiesDiscardedOnLoad() const {
   return this->_load_results.some_data_discarded;
}
constexpr bool DKBoundScriptModel::failedToLoad() const {
   return this->_load_results.failed;
}