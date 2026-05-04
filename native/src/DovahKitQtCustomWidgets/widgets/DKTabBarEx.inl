#pragma once
#include "./DKTabBarEx.h"

constexpr QWidget* DKTabBarEx::tabButton(size_t i, ButtonPosition pos) const {
   if (auto* info = this->_tab_info(i))
      return pos == ButtonPosition::LeftSide ? info->buttons.left : info->buttons.right;
   return nullptr;
}
constexpr bool DKTabBarEx::isTabEnabled(size_t i) const {
   if (auto* info = this->_tab_info(i))
      return info->info.enabled;
   return false;
}
constexpr bool DKTabBarEx::isTabVisible(size_t i) const {
   if (auto* info = this->_tab_info(i))
      return info->info.visible;
   return false;
}