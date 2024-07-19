#pragma once
#include <QComboBox>

class DKComboBox : public QComboBox {
   Q_OBJECT;
   public:
      using QComboBox::QComboBox;

      //
      // Control whether the combobox is allowed to enlarge itself to fit 
      // its contents. Note that even if that enlargement is disabled, the 
      // combobox will still enlarge itself to fit its placeholder text if 
      // there is any (we can't prevent this without reimplementing all of 
      // the sizing logic entirely).
      //
      constexpr bool autoResizeEnabled() const noexcept {
         return this->_state.auto_resize_enabled;
      }
      void setAutoResizeEnabled(bool);

      // We have to shim this propery as part of the `autoResizeEnabled` 
      // behavior. See `sizeHint` implementation for info.
      constexpr int minimumContentsLength() const noexcept {
         return this->_state.desired_min_contents_length;
      }
      void setMinimumContentsLength(int characters);

      virtual QSize minimumSizeHint() const override;
      virtual QSize sizeHint() const override;

   protected:
      struct {
         bool auto_resize_enabled         = true;
         int  desired_min_contents_length = 0;
      } _state;
};