#pragma once
#include <limits>
#include <string_view>
#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QWidget>

#if !defined(QT_DESIGNER_LIB)
   class DKPapyrusBoundScriptListPane;
   class DKPapyrusFragmentFunctionModel;
#endif

class DKPapyrusFragmentFunctionPicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText DESIGNABLE true);
   public:
      DKPapyrusFragmentFunctionPicker(QWidget* parent = nullptr);

      static constexpr const size_t maxScriptnameLength   = std::numeric_limits<uint16_t>::max();
      static constexpr const size_t maxFunctionNameLength = std::numeric_limits<uint16_t>::max();

      #if !defined(QT_DESIGNER_LIB)
         QString currentScriptname() const noexcept;
         QString currentFunction() const noexcept;
         void setCurrentScriptname(const QString&);
         void setCurrentScriptname(const std::string_view);
         void setCurrentFunction(const QString&);
         void setCurrentFunction(const std::string_view);
      #endif

      QString headerText() const;
      void setHeaderText(QString);

      void setSourceWidget(DKPapyrusBoundScriptListPane*);
      
   signals:
      void currentScriptnameChanged(const QString&);
      void currentFunctionChanged(const QString&);
      
   protected:
      #if !defined(QT_DESIGNER_LIB)
         DKPapyrusFragmentFunctionModel* _model = nullptr;
      #endif
      struct {
         QLabel*    header     = nullptr;
         QComboBox* scriptname = nullptr;
         QComboBox* function   = nullptr;
      } _subwidgets;
};