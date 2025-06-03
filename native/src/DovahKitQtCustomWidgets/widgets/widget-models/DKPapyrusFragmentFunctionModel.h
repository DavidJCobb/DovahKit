#pragma once
#include <QAbstractItemModel>
#include <QPointer>
#include "../DKPapyrusBoundScriptListPane.h"

class DKPapyrusFragmentFunctionModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      DKPapyrusFragmentFunctionModel(QObject* parent = nullptr);
      ~DKPapyrusFragmentFunctionModel();
      
   protected:
      struct Script {
         QString name; // empty means it's the "NONE" option
         std::vector<QString> functions;

         bool is_none_item() const;
      };

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual int rowCount(const QModelIndex& parent) const override;
            virtual int columnCount(const QModelIndex& item) const override;
         #pragma endregion
         #pragma region Data
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            virtual QVariant data(const QModelIndex& index, int role) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      QModelIndex noneScriptQMI() const;

      QModelIndex userItemQMI() const;
      QString userItemScriptname() const;
      void setUserItemScriptname(QString);

      QModelIndex scriptQMI(QString) const;

   protected:
      std::vector<Script*> _data;
      QPointer<DKPapyrusBoundScriptListPane> _source_widget;
      Script _user_item;

      void _clear(bool emit_signals = true);

      decltype(_data)::iterator _insertion_point_for(const Script& item);

      void _add_scriptname(QString, bool emit_signals = true);
      bool _has_scriptname(QString) const;
      void _remove_scriptname(QString);

      void _add_function(const QModelIndex&, Script&, QString);
      void _update_script_functions(const QModelIndex&, Script&);

      void _reset_from_source();

   public:
      void setSourceWidget(DKPapyrusBoundScriptListPane*);
};