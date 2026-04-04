#pragma once
#include "./DKBreadcrumbBar.h"

#pragma region Properties
   constexpr Qt::CaseSensitivity DKBreadcrumbBar::caseSensitivity() const noexcept {
      return this->_text_editing.case_sensitivity;
   }
   constexpr bool DKBreadcrumbBar::textEditingAllowed() const noexcept {
      return this->_text_editing.allowed;
   }
   constexpr QChar DKBreadcrumbBar::textSeparator() const noexcept {
      return this->_text_editing.separator;
   }
#pragma endregion
#pragma region State
   constexpr bool DKBreadcrumbBar::areAnySegmentsHidden() const noexcept {
      return this->_state.last_layout.count_shown < this->_segments.size();
   }
   constexpr size_t DKBreadcrumbBar::segmentCount() const noexcept {
      return this->_segments.size();
   }
   constexpr size_t DKBreadcrumbBar::visibleSegmentCount() const noexcept {
      return this->_state.last_layout.count_shown;
   }
#pragma endregion