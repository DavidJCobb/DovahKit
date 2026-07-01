#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <QAbstractItemModel>
namespace nifDK {
   class file;
}

class SkeletonBonesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      SkeletonBonesModel(QObject* parent = nullptr);

   protected:
      struct known_bone {
         std::string name;
         QString     display_name;
      };

   public:
      static constexpr const size_t ColumnCount = 1;
      
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

   protected:
      std::unique_ptr<nifDK::file> _load_nif(std::filesystem::path&&);
      void _make_base_node_item(const std::string& name);

   public:
      constexpr const std::string& baseNodeName() const noexcept { return this->_base_node_name; }
      int baseNodeRow() const;
      bool hasBone(std::string_view) const noexcept;

      void resetFromSkeleton(std::string_view path_relative_to_meshes);

   protected:
      std::vector<known_bone> _data;
      std::string _base_node_name;
};