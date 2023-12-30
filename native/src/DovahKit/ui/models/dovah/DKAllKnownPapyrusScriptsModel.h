#pragma once
#include <QAbstractItemModel>
#include <QHash>
#include <QString>

namespace dovah {
   class compiled_papyrus_script;
}

class DKAllKnownPapyrusScriptsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct known_script {
         QString scriptname;
         QString docstring;
         struct {
            bool conditional = false;
            bool hidden      = false;
         } flags;
         bool is_compiled = false;
      };

   protected:
      using working_collection_type = QHash<QString, known_script>;

   public:
      DKAllKnownPapyrusScriptsModel(QObject* parent) : QAbstractItemModel(parent) {}
      ~DKAllKnownPapyrusScriptsModel();
      
      void clear();
      void populate();

   protected:
      QVector<known_script> _scripts;

      static void _scanPex(working_collection_type& dst, const void* src_data, const size_t src_size);

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent) const override final;
            virtual int         columnCount(const QModelIndex& item) const override final;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
      #pragma endregion
};