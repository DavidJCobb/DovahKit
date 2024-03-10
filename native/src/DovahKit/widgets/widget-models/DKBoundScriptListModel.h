#pragma once
#include <string>
#include <QAbstractItemModel>
#include "dovah/forms/components/papyrus/script_status.h"
#include "./bound-scripts/_forward_declarations.h"

namespace dovah {
   namespace loaded_forms {
      namespace components::papyrus {
         class attachment_data;
         class attached_script;
      }
      class Form;
   }
}
class DKBoundScriptModel;

class DKBoundScriptListModel : public QAbstractItemModel {
   Q_OBJECT;
   protected:
      using vmad_data          = dovah::loaded_forms::components::papyrus::attachment_data;
      using vmad_script_status = ui::bound_script_models::vmad::script_status;
   public:
      DKBoundScriptListModel(QObject* parent = nullptr);
      ~DKBoundScriptListModel();

   protected:
      struct bound_script {
         QString            name;
         vmad_script_status status;
         bool               has_local_properties = false;
         bool               inherited = false;

         // lazy-create this the first time the user opens the properties window for a script.
         // if they click OK, then retain it and reuse it; if they click Cancel, delete it.
         // 
         //  - NOTE: if the user locally defines or clears any properties in an inherited 
         //          script, then force the script status to `overrides_base`.
         //
         // once the user commits all changes to the form, just commit changes one script 
         // model at a time.
         //
         // if the user removes an inherited script, toss its model and use the `status` to 
         // track that it's been removed.
         DKBoundScriptModel* model = nullptr;
      };

      std::vector<bound_script*> _scripts;

   protected:
      const bound_script* _script(const QModelIndex&) const;
      bound_script* _script(const QModelIndex&);

   public slots:
      void clear();

   public:
      void initializeFrom(vmad_data& local);
      void initializeFrom(vmad_data& local, vmad_data& inherited);
      void commitTo(vmad_data& target, dovah::loaded_forms::Form& working_copy);
      
      #pragma region QAbstractItemModel overrides
         virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         virtual QModelIndex parent(const QModelIndex& index) const override;
         virtual int rowCount(const QModelIndex& parent) const override;
         virtual int columnCount(const QModelIndex& item) const override;
         virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         virtual QVariant data(const QModelIndex& index, int role) const override;
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      // Creates a script model for the given script. If the given script already has a model, 
      // creates a copy. You can use this to open a dialog to edit the given script's properties: 
      // create the model; if the user clicks OK, use `replaceScriptModelFor` to save it; else, 
      // delete it yourself.
      DKBoundScriptModel* createScriptModelFor(const QModelIndex& script_qmi);

      // Takes ownership of the model.
      bool replaceScriptModelFor(const QModelIndex&, DKBoundScriptModel*);

      // If the script is already present, returns its QMI.
      QModelIndex addScript(QString name);

      // If the script is inherited, then it's flagged as removed and its local data is tossed. 
      // If the script is entirely local, then it's straight-up deleted.
      bool removeScript(const QModelIndex&);

      // If an inherited script has been locally removed, this un-removes it.
      bool undeleteScript(const QModelIndex&);

      std::vector<std::string> getAllBoundScriptNames() const;
};