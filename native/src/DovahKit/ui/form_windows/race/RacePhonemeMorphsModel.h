#pragma once
#include <vector>
#include <QAbstractItemModel>

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_reference_t;
   class form_stub;
}

class RacePhonemeMorphsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      RacePhonemeMorphsModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum type {
            Name,
            Weight,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;
      
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
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

   public:
      void setAllMorphNames(const std::vector<std::string>&); // changes the number of rows
      //
      QString morphName(size_t) const;
      void setMorphName(size_t, QString);

      void setAllMorphWeights(const std::vector<float>&); // DOES NOT change the number of rows. extra values are discarded; if too few values provided, remaining rows set to 0 weight
      //
      float morphWeight(size_t) const;
      void setMorphWeight(size_t, float);

      QModelIndex addMorph();
      void deleteMorph(size_t);
      void deleteMorph(const QModelIndex&);

      [[nodiscard]] std::vector<float> allMorphWeights() const;
      [[nodiscard]] std::vector<QString> allMorphNames() const;

   protected:
      struct Morph {
         QString name;
         float   weight = 0;
      };

      std::vector<Morph> _morphs;

      QString _get_new_morph_name() const;
};