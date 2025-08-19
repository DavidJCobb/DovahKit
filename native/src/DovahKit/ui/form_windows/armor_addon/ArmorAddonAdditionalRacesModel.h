#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "dovah/form_types.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_reference_t;
   class form_stub;
}

// copied andp asted from RaceEquipSlotsModel.
// once we get to Sustain, we should unify the two.
class ArmorAddonAdditionalRacesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      ArmorAddonAdditionalRacesModel(QObject* parent = nullptr);

      static constexpr const size_t ColumnCount = 1;
      static constexpr const dovah::form_type desired_form_type = dovah::form_type::race;
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

   protected:
      void _on_data_acquire();
      void _on_data_abandon_imminent();
      void _on_form_created(dovah::form_stub*);
      void _on_form_modified(dovah::form_stub*);
      void _on_form_deletion_imminent(dovah::form_stub*);

   public:
      void initializeFrom(const std::vector<dovah::form_reference_t>&);
      void commitTo(std::vector<dovah::form_reference_t>& list, dovah::loaded_forms::Form& containing_form) const;

      bool isChecked(size_t row) const;
      bool isChecked(const dovah::form_stub&) const;
      void setChecked(size_t row, bool checked);
      void setChecked(const dovah::form_stub&, bool checked);

   protected:
      struct KnownForm {
         bool operator==(const KnownForm&) const noexcept = default; // *sigh*

         dovah::form_stub* stub = nullptr;
         bool    checked = false;
         QString cached_editor_id;

         static bool sort(const KnownForm&, const KnownForm&);
      };

      std::vector<KnownForm> _data;

      void _insert_item(const KnownForm& item, bool emit_model_sync_signals);
      decltype(_data)::iterator _insertion_point_for(const KnownForm&);
      void _re_sort_item(const KnownForm&, std::optional<QString> prior_name);
};