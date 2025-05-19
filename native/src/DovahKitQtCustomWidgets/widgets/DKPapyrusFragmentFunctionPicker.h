#pragma once
#include <string_view>
#include <QComboBox>
#include <QLabel>
#include <QWidget>

namespace dovah {
   class compiled_papyrus_script;
}

class DKPapyrusFragmentFunctionPicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText DESIGNABLE true);
   public:
      DKPapyrusFragmentFunctionPicker(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         QString currentScriptname() const noexcept;
         QString currentFunction() const noexcept;
         void setCurrentScriptname(const QString&);
         void setCurrentScriptname(const std::string_view);
         void setCurrentFunction(const QString&);
         void setCurrentFunction(const std::string_view);
      
         int scriptnameMaxLength() const noexcept;
         void setScriptnameMaxLength(size_t) noexcept;
         int functionMaxLength() const noexcept;
         void setFunctionMaxLength(size_t) noexcept;
      
         void addScriptname(const QString&);
         void clearAvailableScriptnames();
         void clearCurrentValues();
         void removeScriptname(const QString&);
      #endif

      QString headerText() const;
      void setHeaderText(QString);
      
   signals:
      void currentScriptnameChanged(const QString&);
      void currentFunctionChanged(const QString&);
      
   protected:
      struct script {
         QString name;
         dovah::compiled_papyrus_script* compiled = nullptr;
      };

      QList<script> scripts;
      struct {
         QLabel*    header     = nullptr;
         QComboBox* scriptname = nullptr;
         QComboBox* function   = nullptr;
      } _subwidgets;

      #if !defined(QT_PLUGIN)
      script* _getScriptData(const QString& name);
      script* _getOrCreateScriptData(const QString& name);
      #endif
};