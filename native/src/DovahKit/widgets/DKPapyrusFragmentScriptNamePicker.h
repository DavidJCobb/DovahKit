#pragma once
#include <limits>
#include <string_view>
#include <QComboBox>
#include <QPointer>
#include <QWidget>
#include "./DKPapyrusBoundScriptListPane.h"

class DKPapyrusFragmentScriptNamePicker : public QWidget {
   Q_OBJECT;
   public:
      DKPapyrusFragmentScriptNamePicker(QWidget* parent = nullptr);

      static constexpr const size_t maxScriptnameLength = std::numeric_limits<uint16_t>::max();

      QString value() const noexcept;
      void setValue(const QString&);
      void setValue(const std::string_view);

      DKPapyrusBoundScriptListPane* sourceWidget() const { return this->_source; }
      void setSourceWidget(DKPapyrusBoundScriptListPane*);
      
   signals:
      void sourceWidgetChanged(DKPapyrusBoundScriptListPane*);
      void valueChanged(const QString&);
      
   protected:
      QPointer<DKPapyrusBoundScriptListPane> _source = nullptr;
      struct {
         QComboBox* scriptname = nullptr;
      } _subwidgets;

      void _on_source_script_list_changed();
};