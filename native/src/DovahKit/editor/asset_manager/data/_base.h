#pragma once
#include <type_traits>
#include <QString>
#include "../asset.h"
#include "../asset_manager.h"

namespace dovah {
   class bsa_archived_file;
   class form_stub;
}
class DovahKitAsset;

class DovahKitAssetData {
   public:
      DovahKitAssetData(DovahKitAsset& o) : owner(o) {}
      virtual ~DovahKitAssetData() {}

      DovahKitAsset& owner;
      size_t pending_count = 0;

      virtual bool load(const dovah::bsa_archived_file&) = 0;
      virtual bool load(dovah::form_stub&) = 0;

      virtual bool hasPendingDependencies() const { return false; }
      virtual void requestDependencies() {}

      void dependencyResolved();

   protected:
      //
      // Boilerplate to help with asset dependencies. The general approach you'd take is to have two 
      // members on your data subclass:
      // 
      //  - A pointer to a list of QStrings, representing the paths of dependency assets. The "list" 
      //    should be a std::array or std::vector wrapped in a struct, e.g.
      // 
      //       struct path_list {
      //          std::array<QString, 4> paths = {};
      //       };
      // 
      //       path_list* pending = nullptr;
      // 
      //    A wrapper struct is preferred in order to help avoid the risk of forgetting to initialize 
      //    an std::array.
      // 
      //  - An array or vector of DovahKitAssetReceptors, meant to refer to the dependency assets 
      //    after they've been loaded.
      // 
      // During the initial load, you'd gather and store paths in the former list (heap-allocating it 
      // if it doesn't exist yet). Then, hasPendingDependencies should return true if the list exists 
      // and contains any non-empty path strings.
      // 
      // Then, in your reqpestDependencies overload, you can just call _requestDependencies and pass 
      // the former pointer and the latter list directly.
      //
      template<typename PathList, typename ReceptorList> requires requires(PathList x, ReceptorList y) {
         { x.paths.size() } -> std::same_as<size_t>;
         { y.size() } -> std::same_as<size_t>;
         { x.paths[0] } -> std::same_as<QString&>;
         { y[0] } -> std::same_as<DovahKitAssetReceptor&>;
      }
      void _requestDependencies(PathList*& pl, ReceptorList& rl) {
         if (!pl)
            return; // no dependencies
         //
         PathList* list = pl;
         pl = nullptr;
         //
         for (size_t i = 0; i < list->paths.size(); ++i) {
            if (list->paths[i].isEmpty())
               continue;
            ++this->pending_count;
            //
            QObject::connect(&rl[i], &DovahKitAssetReceptor::ready,  &this->owner, [this]() { this->dependencyResolved(); });
            QObject::connect(&rl[i], &DovahKitAssetReceptor::failed, &this->owner, [this]() { this->dependencyResolved(); });
         }
         if (this->pending_count) {
            auto& am = DovahKitAssetManager::get();
            for (size_t i = 0; i < rl.size(); ++i) {
               const auto& p = list->paths[i];
               if (p.isEmpty())
                  continue;
               rl[i] = am.requestAsset(p);
            }
         }
         //
         delete list;
      }
};
