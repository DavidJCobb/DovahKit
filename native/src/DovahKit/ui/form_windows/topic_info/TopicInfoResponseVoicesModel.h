#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/game_file_path.h"
namespace dovah {
   class form_stub;
}

class TopicInfoResponseVoicesModel : public QAbstractItemModel {
   protected:
      struct node_type {
         dovah::form_stub* voicetype = nullptr;
         struct {
            QString path;
            ui::types::game_file_path scoped_path;
            bool    exists = false;
         } audio_file;
         struct {
            QString voicetype_editor_id;
         } cached;

         void set_audio_file_path(const std::string&);
      };

   public:
      using ResponseUID = uint8_t;

      struct VoiceFileLocationInfo {
         std::string data_filename;
         std::string quest_editor_id;
         std::string topic_editor_id;
         uint32_t    info_form_id = 0;
         ResponseUID response_uid = 0;
      };

      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Voicetype,
            FilePath,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      TopicInfoResponseVoicesModel(QObject* parent);
      
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

      void setVoiceFileLocationInfo(VoiceFileLocationInfo&&);

   protected:
      std::vector<node_type> _data;
      VoiceFileLocationInfo _voice_file_location_info;

      static bool _compare_for_sort(const node_type&, const node_type&);
      decltype(_data)::iterator _insertion_point_for(const node_type&);
      void _re_sort_item(size_t row);

      void _insert_voicetype(dovah::form_stub& voicetype, bool emit_signals);
      void _on_active_file_saved();
      void _pull_all_voicetypes();
};