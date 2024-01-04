#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractItemModel>
#include <QString>
#include <QVector>
#include "dovah/forms/components/papyrus.h"

class DKPapyrusScriptObjectListModel : public QAbstractTableModel {
   Q_OBJECT;
   public:
      using working_copy_type = dovah::loaded_forms::Form;

      using vmad_data     = dovah::loaded_forms::components::papyrus::attachment_data;
      using vmad_script   = dovah::loaded_forms::components::papyrus::attached_script;
      using vmad_property = dovah::loaded_forms::components::papyrus::property;

      using script_status = dovah::loaded_forms::components::papyrus::script_status;

   public:
      class Script {
         public:
            QString name;
            struct {
               std::optional<script_status> parent;
               std::optional<script_status> target;
            } statuses;
            bool properties_set_on_target = false;

            std::optional<script_status> getComputedStatus() const;
            bool nameMatches(QString) const;
      };

      struct raw_script_info {
         dovah::form_stub* attached_to = nullptr;
         vmad_script* parent_script = nullptr;
         vmad_script* target_script = nullptr;

         constexpr bool is_inherited_and_removed() const {
            if (!attached_to || !parent_script || !target_script)
               return false;
            return (target_script->status == script_status::removed);
         }
      };
      
   protected:
      working_copy_type* attached_to = nullptr; // form working copy
      struct {
         vmad_data* parent = nullptr;
         vmad_data* target = nullptr;
      } vmads;
      QVector<Script> scripts;
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      
   public:
      DKPapyrusScriptObjectListModel(QObject* parent = nullptr);
      ~DKPapyrusScriptObjectListModel() {
         this->clearWorkingVMAD();
      }

      void setWorkingVMAD(working_copy_type&, vmad_data& target);
      void setWorkingVMAD(working_copy_type&, vmad_data& target, vmad_data& parent);
      void clearWorkingVMAD();
      void syncToWorkingVMAD();
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      const raw_script_info row(int rowIndex) const noexcept;

      QModelIndex index(QString scriptname) const;
      
   public slots:
      QModelIndex addScript(QString scriptname); // if the script is already present, returns its QMI
      void removeScript(int index);
      void undeleteInheritedScript(int index);

      void refreshScript(QString scriptname); // handle changes made to working-copy VMAD by DKPapyrusScriptObjectModel
};