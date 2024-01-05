#pragma once
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include "dovah/forms/components/papyrus/property_status.h"
#include "dovah/forms/components/papyrus/property_type.h"
#include "dovah/forms/components/papyrus/script_status.h"
#include "dovah/core.h"

namespace dovah {
   namespace loaded_forms {
      namespace components::papyrus {
         class attachment_data;
         class attached_script;
         class property;
      }
      class Form;
   }
   class form_stub;
}

/*//

   DKFormVMADModel

   This is a single model that can represent all script data attached to a given form or 
   alias, taking the base form (when dealing with refs) into account for display purposes. 
   Each script is represented as a top-level model node; each property on a script is 
   represented as a child node.

   The intended use case is as follows:

   * Pass the target form-working-copy and VMAD components into the model. Make the user's 
     edits to the model data. Then, tell the model to "commit" the data in order to edit 
     the content actually in the form.

   * For a list or table view of all scripts on a form, use an invalid QModelIndex as the 
     QAbstractItemView's root node.

   * For a list or table view of all properties on a script, use the script's QModelIndex 
     as the QAbstractItemView's root node.

//*/

class DKFormVMADModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      DKFormVMADModel(QObject* parent = nullptr);
      ~DKFormVMADModel();

      struct ScriptColumn {
         enum {
            Name,
            _Count
         };
      };
      static constexpr const size_t ScriptColumnCount = ScriptColumn::_Count;

      struct PropertyColumn {
         enum {
            Name,
            Type,
            Value,
            _Count
         };
      };
      static constexpr const size_t PropertyColumnCount = PropertyColumn::_Count;

      struct ScriptMetadata {
         bool attached_on_parent    = false;
         bool attached_on_target    = false;
         bool inherited_and_removed = false;
      };

   public:
      using working_copy_type = dovah::loaded_forms::Form;

      using vmad_data     = dovah::loaded_forms::components::papyrus::attachment_data;
      using vmad_script   = dovah::loaded_forms::components::papyrus::attached_script;
      using vmad_property = dovah::loaded_forms::components::papyrus::property;

      using property_type = dovah::loaded_forms::components::papyrus::property_type;

      using script_status   = dovah::loaded_forms::components::papyrus::script_status;
      using property_status = dovah::loaded_forms::components::papyrus::property_status;
      
      struct object_property_value {
         static constexpr const uint16_t no_alias = 0xFFFF;

         dovah::form_stub* form     = nullptr;
         uint16_t          alias_id = no_alias;
      };

      using property_value = std::variant<
         std::monostate, // only for clearing an inherited property value REFR-side
         //
         object_property_value,
         QString,
         int32_t,
         float,
         bool,
         //
         std::vector<object_property_value>,
         std::vector<QString>,
         std::vector<int32_t>,
         std::vector<float>,
         std::vector<bool>
      >;

   protected:
      class Property {
         public:
            struct Typeinfo {
               property_type underlying_type;
               QString name; // scriptname, pulled from the compiled PEX, when `underlying_type` is Form or Form[]
            };
            struct Binding {
               property_status status;
               property_value  value;

               property_type typecode() const;
            };

         public:
            QString name;
            QString docstring;
            std::optional<Typeinfo> type; // type as dictated by the compiled script; absent if the PEX is not loadable
            struct {
               std::optional<Binding> parent; // base form, if the form we're currently editing is a REFR
               std::optional<Binding> target; // form we're currently editing. NOTE: should have a value if clearing an inherited property value REFR-side!
            } bindings;
            //
            QString value_string; // cached; computed from `bindings`

            void clearParentBinding();
            void clearTargetBinding();
            void setParentBinding(const vmad_property&);
            void setTargetBinding(const vmad_property&);
            void setBindings(const vmad_property& parent, const vmad_property& target);

            bool valueTypeIsOrContainsForm() const;
            bool isOrContainsForm(const dovah::form_stub&) const;
            QString typeString() const;
            void recacheValueString();
            bool onFormDeletionImminent(const dovah::form_stub&); // returns true if anything about this property has changed

            // Used when the user wants to set this property's value. Should change the "target" binding, forcing the 
            // status to `defined_locally` and setting the value.
            void changeValueTo(const property_value&);

            std::optional<property_status> getComputedStatus() const;
            bool nameMatches(QString) const;
      };
      class Script {
         public:
            ~Script();

            QString name;
            struct {
               std::optional<script_status> parent;
               std::optional<script_status> target;
            } statuses;
            bool properties_set_on_target = false;
            QVector<Property*> properties;

            void loadPropertiesFromPex();
            void loadParentPropertyData(const vmad_property&);
            void loadTargetPropertyData(const vmad_property&);

            Property* lookupProperty(QString name);

            std::optional<script_status> getComputedStatus() const;
            bool nameMatches(QString) const;
      };

   protected:
      working_copy_type* attached_to = nullptr; // form working copy
      struct {
         vmad_data* parent = nullptr;
         vmad_data* target = nullptr;
      } vmads;
      QVector<Script*> scripts;

      dovah::form_stub* _getBaseForm() const;
      Script* _getContainingScript(QModelIndex prop_qmi);
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formModified(dovah::form_stub*);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();

   public:
      void setWorkingVMAD(working_copy_type&, vmad_data& target);
      void setWorkingVMAD(working_copy_type&, vmad_data& target, vmad_data& parent);
      void commitToWorkingVMAD();
      void unsetWorkingVMAD();

      bool indexIsScript(const QModelIndex) const;
      bool indexIsProperty(const QModelIndex) const;

      // Functors should take QModelIndex and pointer-to-data as args.
      // Method returns `false` if QMI doesn't match any model data.
      template<typename ScriptFunctor, typename PropFunctor>
      bool handleIndexByType(QModelIndex qmi, ScriptFunctor&& sf, PropFunctor&& pf);
      //
      template<typename ScriptFunctor, typename PropFunctor>
      bool handleIndexByType(QModelIndex qmi, ScriptFunctor&& sf, PropFunctor&& pf) const;
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      const ScriptMetadata getScriptMetadata(QModelIndex) const noexcept;

   public:
      QModelIndex addScript(QString scriptname); // if the script is already present, returns its QMI
      void removeScript(QModelIndex);
      void undeleteInheritedScript(QModelIndex);

      QModelIndex scriptIndex(QString name) const;
      QModelIndex propertyIndex(QModelIndex script_qmi, QString name) const;

      void clearPropertyValue(QModelIndex); // use script default
      void revertPropertyValue(QModelIndex); // revert to inherited value
      void setPropertyValue(QModelIndex, const property_value&);
};

#include "./DKFormVMADModel.inl"