#pragma once
#include <QAbstractItemModel>
#include "dovah/forms/components/papyrus/attached_script.h"
#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/components/papyrus/property.h"
#include "dovah/forms/components/papyrus/property_type.h"
#include "dovah/forms/components/papyrus/script_status.h"
#include "./bound-scripts/_forward_declarations.h"
#include "./bound-scripts/property.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

class DKBoundScriptModel : public QAbstractItemModel {
   Q_OBJECT;
   protected:
      using property = ui::bound_script_models::property;
   public:
      using vmad_attachment_data = ui::bound_script_models::vmad::attachment_data;
      using vmad_script          = ui::bound_script_models::vmad::attached_script;

      using PropertyValue = ui::bound_script_models::property_value;
      
      struct Column {
         enum {
            Name,
            Type,
            Value,
            _Count
         };
      };
      static constexpr const size_t ColumnCount = Column::_Count;

   protected:
      using vmad_property       = ui::bound_script_models::vmad::property;
      using vmad_property_value = ui::bound_script_models::vmad::property_value;

   public:
      DKBoundScriptModel(QString scriptname, QObject* parent = nullptr);
      DKBoundScriptModel(const DKBoundScriptModel&, QObject* parent = nullptr);
      ~DKBoundScriptModel();
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formModified(dovah::form_stub*);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();

   protected:
      void _clear();
      void _load_property_definitions_from(std::string_view scriptname);

   protected:
      const QString          _scriptname;
      std::vector<property*> _properties;
      struct {
         bool any_properties_defined_locally = false;
      } _cached;
      struct {
         bool failed              = false; // any PEX files failed to load?
         bool some_data_discarded = false; // true if any properties were invalid / not in the PEX / etc.
      } _load_results;
      struct {
         bool inherited = false;
         bool cleared   = false; // inherited, but removed locally
      } _status;

   protected:
      void _emit_row_changed(const QModelIndex&);

      property* _lookup_property(std::string_view);

      const property* _property(const QModelIndex&) const;
      property* _property(const QModelIndex&);
      
      static std::optional<dovah::form_type> _guess_object_property_type(const vmad_property_value&);
      std::optional<PropertyValue> _load_property_value(const vmad_property&, const property& info); // Returns empty on failure.

      void _update_any_properties_local();

   public:
      void initializeFrom(const vmad_script& local_script, const vmad_attachment_data& parent_vmad);
      void initializeFrom(const vmad_script& local_script);
      void commitTo(vmad_script& local_script, dovah::loaded_forms::Form& working_copy);
      
      #pragma region QAbstractItemModel overrides
         virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         virtual QModelIndex parent(const QModelIndex& index) const override;
         virtual int rowCount(const QModelIndex& parent) const override;
         virtual int columnCount(const QModelIndex& item) const override;
         virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         virtual QVariant data(const QModelIndex& index, int role) const override;
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      constexpr bool anyPropertiesDefinedLocally() const;
      constexpr bool anyPropertiesDiscardedOnLoad() const;
      constexpr bool failedToLoad() const;

      void autoFillProperty(const QModelIndex&);
      void clearProperty(const QModelIndex&);
      void makePropertyLocal(const QModelIndex&);
      void revertProperty(const QModelIndex&);
      void setPropertyLocalValue(const QModelIndex&, const PropertyValue&);
      void setPropertyLocalValueElement(const QModelIndex&, const PropertyValue&, size_t array_index); // fails (does nothing) on non-array properties

      void autoFillAllProperties();

      std::optional<PropertyValue> getPropertyValue(const QModelIndex&, bool local_only = false) const;
      std::optional<PropertyValue> getPropertyValueElement(const QModelIndex&, size_t array_index, bool local_only = false) const; // fails (empty result) on non-array properties
};

#include "./DKBoundScriptModel.inl"