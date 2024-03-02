#pragma once
#include <vector>
#include <QAbstractItemModel>
#include "helpers/singleton_ex.h"
#include "dovah/form_types.h"

namespace dovahkit::subsystems::papyrus {
   class known_script;
}

class DKAllPapyrusScriptsModel;
class DKAllPapyrusScriptsModel : public QAbstractItemModel, public cobb::singleton_ex<DKAllPapyrusScriptsModel> {
   Q_OBJECT;
   protected:
      using known_script     = dovahkit::subsystems::papyrus::known_script;
      using known_script_ptr = const known_script*;

   public:
      static constexpr const auto IsHiddenRole      = (Qt::ItemDataRole)(Qt::UserRole);
      static constexpr const auto IsConditionalRole = (Qt::ItemDataRole)(Qt::UserRole + 1);

   protected:
      DKAllPapyrusScriptsModel();

   protected:
      struct Script {
         QString          name;
         QString          docstring;
         known_script_ptr info = nullptr;
      };

      std::vector<Script*> _scripts;

   protected:
      void _clear(bool silent = false);
      void _gatherScriptnamesOnLoadingDone();

   public:
      bool scriptIsAttachableTo(const QModelIndex&, dovah::form_type_t) const;

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