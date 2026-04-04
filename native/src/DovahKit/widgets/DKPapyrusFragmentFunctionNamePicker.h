#pragma once
#include <limits>
#include <string_view>
#include <QComboBox>
#include <QPointer>
#include <QWidget>
#include "./DKPapyrusFragmentScriptNamePicker.h"

class DKPapyrusFragmentFunctionModel;

class DKPapyrusFragmentFunctionNamePicker : public QWidget {
   Q_OBJECT;
   public:
      DKPapyrusFragmentFunctionNamePicker(QWidget* parent = nullptr);
      
      static constexpr const size_t maxFunctionNameLength = std::numeric_limits<uint16_t>::max();

      QString value() const noexcept;
      void setValue(const QString&);
      void setValue(const std::string_view);

      void setSourceWidget(DKPapyrusFragmentScriptNamePicker*);
      
   signals:
      void valueChanged(const QString&);
      
   protected:
      QPointer<DKPapyrusFragmentScriptNamePicker> _source = nullptr;
      DKPapyrusFragmentFunctionModel* _model = nullptr;
      struct {
         QComboBox* function = nullptr;
      } _subwidgets;
};