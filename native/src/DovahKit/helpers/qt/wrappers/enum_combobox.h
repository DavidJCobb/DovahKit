#pragma once
#include <QComboBox>

namespace cobb::qt::wrappers {
   template<typename Enum>
   class EnumCombobox {
      public:
         using value_type = Enum;

      protected:
         QComboBox* _widget = nullptr;

      public:
         constexpr EnumCombobox() {}
         EnumCombobox(QComboBox* w) : _widget(w) {}

         operator QComboBox* () const { return _widget; }

         constexpr QComboBox* widget() const noexcept { return this->_widget; }

         void addItem(const QString& text, value_type value) {
            widget()->addItem(text, (int)value);
         }

         template<class TranslateVia = QObject, size_t Count>
         void addItems(const std::array<std::pair<value_type, const char*>, Count>& items, const char* translation_disambiguator = nullptr) {
            for (const auto& item : items) {
               auto text = TranslateVia::tr(item.first, translation_disambiguator);
               addItem(text, item.second);
            }
         }

         void beginOneWaySync(value_type& target) {
            this->setValue(target);
            //
            auto* w = widget();
            QObject::connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged), w, [w, &target](int index) {
               target = (value_type)w->currentData().toInt();
            });
         }

         value_type value() const noexcept {
            auto data = widget()->currentData();
            auto i    = data.toInt();
            return (value_type)i;
         }
         bool setValue(value_type v, value_type default_value = {}) {
            auto i = widget()->findData((int)v);
            if (i >= 0) {
               widget()->setCurrentIndex(i);
               return true;
            }
            if (value != default_value) {
               i = widget()->findData((int)default_value);
            }
            if (i < 0) {
               i = 0;
            }
            widget()->setCurrentIndex(i);
            return false;
         }

         bool setValueSilent(value_type v, value_type default_value = {}) {
            const auto blocker = QSignalBlocker(widget());
            return setValue(v, default_value);
         }
   };
}
