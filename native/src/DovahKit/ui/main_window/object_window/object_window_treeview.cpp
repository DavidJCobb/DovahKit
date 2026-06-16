#include "./object_window_treeview.h"
#include <algorithm>
#include "dovah/form_stub_addenda.h"
#include "editor/core.h"
#include "editor/subsystems/form_info_cache/cacheable_traits/model_path.h"
#include "editor/subsystems/form_info_cache/core.h"

#pragma region ObjectWindowTreeItem
ObjectWindowTreeItem::~ObjectWindowTreeItem() {
   this->clear();
}

/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_filter(const QString& name) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = type_t::filter;
   return *item;
}
/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_top_level(const QString& name) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = type_t::top_level;
   return *item;
}
/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_form_type(const QString& name, dovah::form_type ft) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = type_t::top_level;
   item->form_type = ft;
   return *item;
}

ObjectWindowTreeItem& ObjectWindowTreeItem::appendChild(ObjectWindowTreeItem& child) {
   if (child.parent)
      child.parent->takeChild(child);
   this->children.push_back(&child);
   child.parent = this;
   return *this;
}
void ObjectWindowTreeItem::takeChild(ObjectWindowTreeItem& child) {
   if (child.parent != this)
      return;
   auto i = this->children.indexOf(&child);
   if (i >= 0)
      this->children.remove(i);
}

[[nodiscard]] int ObjectWindowTreeItem::indexOf(const QString& name) const noexcept {
   auto& list = this->children;
   auto  size = list.size();
   for (int i = 0; i < size; ++i)
      if (list[i]->name.compare(name, Qt::CaseInsensitive) == 0)
         return i;
   return -1;
}

ObjectWindowTreeItem* ObjectWindowTreeItem::findChildByFormType(dovah::form_type ft) const noexcept {
   for (auto* child : this->children) {
      if (child->form_type == ft)
         return child;
      if (child->type == type_t::filter)
         continue;
      auto* result = child->findChildByFormType(ft);
      if (result)
         return result;
   }
   return nullptr;
}

void ObjectWindowTreeItem::clear() {
   for (auto* child : this->children)
      delete child;
   this->children.clear();
}
dovah::form_type ObjectWindowTreeItem::containingFormType() const noexcept {
   auto* node = this;
   do {
      auto ft = node->form_type;
      if (ft != dovah::form_type::none && ft.has_value())
         return ft.value();
   } while (node = node->parent);
   return dovah::form_type::none;
}
void ObjectWindowTreeItem::gatherFormTypes(QVector<dovah::form_type>& out) const noexcept {
   if (this->type == type_t::filter) {
      auto ft = this->containingFormType();
      if (!out.contains(ft))
         out.push_back(ft);
      return;
   }
   if (this->form_type.has_value()) {
      if (!out.contains(this->form_type.value()))
         out.push_back(this->form_type.value());
      return;
   }
   for (auto* child : this->children)
      child->gatherFormTypes(out);
}
void ObjectWindowTreeItem::recursiveSort() {
   this->sort();
   for (auto* item : this->children)
      item->recursiveSort();
}
void ObjectWindowTreeItem::sort() {
   auto& list = this->children;
   std::sort(list.begin(), list.end(), [](const ObjectWindowTreeItem* a, const ObjectWindowTreeItem* b) {
      return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
   });
}
#pragma endregion

#pragma region ObjectWindowTreeModel
   ObjectWindowTreeModel::ObjectWindowTreeModel(QObject* parent) : QAbstractItemModel(parent) {
      this->_nodes.root = new item_type;
      this->_nodes.root->type = item_type::type_t::root;
      //
      this->beginResetModel();
      #pragma region Build contents
      constexpr const char* disambig = "object window";
      //
      #pragma region Build notable nodes
         this->_nodes.all    = &item_type::make_top_level(tr("All", disambig));
         this->_nodes.quests = &item_type::make_form_type(tr("Quest", disambig), dovah::form_type::quest);
      #pragma endregion
      //
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Actors", disambig))
            .appendChild(item_type::make_form_type(tr("ActorBase", disambig), dovah::form_type::actor_base))
            .appendChild(item_type::make_form_type(tr("Actor Action", disambig), dovah::form_type::action))
            .appendChild(item_type::make_form_type(tr("BodyPartData", disambig), dovah::form_type::body_part_data))
            .appendChild(item_type::make_form_type(tr("LeveledCharacter", disambig), dovah::form_type::leveled_character))
            .appendChild(item_type::make_form_type(tr("Perk", disambig), dovah::form_type::perk))
            .appendChild(item_type::make_form_type(tr("TalkingActivator", disambig), dovah::form_type::talking_activator))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Audio", disambig))
            .appendChild(item_type::make_form_type(tr("Acoustic Space", disambig), dovah::form_type::acoustic_space))
            .appendChild(item_type::make_form_type(tr("Music Track", disambig), dovah::form_type::music_track))
            .appendChild(item_type::make_form_type(tr("Music Type", disambig), dovah::form_type::music_type))
            .appendChild(item_type::make_form_type(tr("Reverb Parameters", disambig), dovah::form_type::reverb_parameters))
            .appendChild(item_type::make_form_type(tr("Sound Category", disambig), dovah::form_type::sound_category))
            .appendChild(item_type::make_form_type(tr("Sound Descriptor", disambig), dovah::form_type::sound_descriptor))
            .appendChild(item_type::make_form_type(tr("Sound Marker", disambig), dovah::form_type::sound))
            .appendChild(item_type::make_form_type(tr("Sound Output Model", disambig), dovah::form_type::sound_output_model))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Character", disambig))
            .appendChild(item_type::make_form_type(tr("Association Type", disambig), dovah::form_type::association_type))
            .appendChild(item_type::make_form_type(tr("Class", disambig), dovah::form_type::combat_class))
            .appendChild(item_type::make_form_type(tr("Equip Slot", disambig), dovah::form_type::equip_slot))
            .appendChild(item_type::make_form_type(tr("Faction", disambig), dovah::form_type::faction))
            .appendChild(item_type::make_form_type(tr("HeadPart", disambig), dovah::form_type::head_part))
            .appendChild(item_type::make_form_type(tr("Movement Type", disambig), dovah::form_type::movement_type))
            .appendChild(item_type::make_form_type(tr("Package", disambig), dovah::form_type::package))
            .appendChild(*this->_nodes.quests)
            .appendChild(item_type::make_form_type(tr("Race", disambig), dovah::form_type::race))
            .appendChild(item_type::make_form_type(tr("Relationship", disambig), dovah::form_type::relationship))
            .appendChild(item_type::make_form_type(tr("SM Event Node", disambig), dovah::form_type::story_event_node))
            .appendChild(item_type::make_form_type(tr("VoiceType", disambig), dovah::form_type::voicetype))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Items", disambig))
            .appendChild(item_type::make_form_type(tr("Ammo", disambig), dovah::form_type::ammo))
            .appendChild(item_type::make_form_type(tr("Apparatus", disambig), dovah::form_type::apparatus))
            .appendChild(item_type::make_form_type(tr("Armor", disambig), dovah::form_type::armor))
            .appendChild(item_type::make_form_type(tr("ArmorAddon", disambig), dovah::form_type::armor_addon))
            .appendChild(item_type::make_form_type(tr("Book", disambig), dovah::form_type::book))
            .appendChild(item_type::make_form_type(tr("Constructible Object", disambig), dovah::form_type::constructible_object))
            .appendChild(item_type::make_form_type(tr("Ingredient", disambig), dovah::form_type::ingredient))
            .appendChild(item_type::make_form_type(tr("Key", disambig), dovah::form_type::key))
            .appendChild(item_type::make_form_type(tr("LeveledItem", disambig), dovah::form_type::leveled_item))
            .appendChild(item_type::make_form_type(tr("MiscItem", disambig), dovah::form_type::misc_item))
            .appendChild(item_type::make_form_type(tr("Note", disambig), dovah::form_type::note))
            .appendChild(item_type::make_form_type(tr("Outfit", disambig), dovah::form_type::outfit))
            .appendChild(item_type::make_form_type(tr("Soul Gem", disambig), dovah::form_type::soul_gem))
            .appendChild(item_type::make_form_type(tr("Weapon", disambig), dovah::form_type::weapon))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Magic", disambig))
            .appendChild(item_type::make_form_type(tr("Dual Cast Data", disambig), dovah::form_type::dual_cast_data))
            .appendChild(item_type::make_form_type(tr("Enchantment", disambig), dovah::form_type::enchantment))
            .appendChild(item_type::make_form_type(tr("LeveledSpell", disambig), dovah::form_type::leveled_spell))
            .appendChild(item_type::make_form_type(tr("Magic Effect", disambig), dovah::form_type::magic_effect))
            .appendChild(item_type::make_form_type(tr("Potion", disambig), dovah::form_type::potion))
            .appendChild(item_type::make_form_type(tr("Scroll", disambig), dovah::form_type::scroll))
            .appendChild(item_type::make_form_type(tr("Shout", disambig), dovah::form_type::shout))
            .appendChild(item_type::make_form_type(tr("Spell", disambig), dovah::form_type::spell))
            .appendChild(item_type::make_form_type(tr("Word of Power", disambig), dovah::form_type::word_of_power))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Miscellaneous", disambig))
            .appendChild(item_type::make_form_type(tr("Actor Value", disambig), dovah::form_type::actor_value_info))
            .appendChild(item_type::make_form_type(tr("Animation Prop", disambig), dovah::form_type::animation_prop))
            .appendChild(item_type::make_form_type(tr("Art Object", disambig), dovah::form_type::art_object))
            .appendChild(item_type::make_form_type(tr("Collision Layer", disambig), dovah::form_type::collision_layer))
            .appendChild(item_type::make_form_type(tr("Color", disambig), dovah::form_type::color))
            .appendChild(item_type::make_form_type(tr("Combat Style", disambig), dovah::form_type::combat_style))
            .appendChild(item_type::make_form_type(tr("FormList", disambig), dovah::form_type::formlist))
            .appendChild(item_type::make_form_type(tr("Global", disambig), dovah::form_type::global))
            .appendChild(item_type::make_form_type(tr("Idle Marker", disambig), dovah::form_type::idle_marker))
            .appendChild(item_type::make_form_type(tr("Keyword", disambig), dovah::form_type::keyword))
            .appendChild(item_type::make_form_type(tr("Land Texture", disambig), dovah::form_type::land_texture))
            .appendChild(item_type::make_form_type(tr("Loading Screen", disambig), dovah::form_type::loading_screen))
            .appendChild(item_type::make_form_type(tr("Material Object", disambig), dovah::form_type::material_object))
            .appendChild(item_type::make_form_type(tr("Message", disambig), dovah::form_type::message))
            .appendChild(item_type::make_form_type(tr("TextureSet", disambig), dovah::form_type::texture_set))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("Special Effects", disambig))
            .appendChild(item_type::make_form_type(tr("Add-on Node", disambig), dovah::form_type::addon_node))
            .appendChild(item_type::make_form_type(tr("Camera Shot", disambig), dovah::form_type::camera_shot))
            .appendChild(item_type::make_form_type(tr("Debris", disambig), dovah::form_type::debris))
            .appendChild(item_type::make_form_type(tr("EffectShader", disambig), dovah::form_type::effect_shader))
            .appendChild(item_type::make_form_type(tr("Explosion", disambig), dovah::form_type::explosion))
            .appendChild(item_type::make_form_type(tr("Footstep", disambig), dovah::form_type::footstep))
            .appendChild(item_type::make_form_type(tr("Footstep Set", disambig), dovah::form_type::footstep_set))
            .appendChild(item_type::make_form_type(tr("Hazard", disambig), dovah::form_type::hazard))
            .appendChild(item_type::make_form_type(tr("Imagespace", disambig), dovah::form_type::imagespace))
            .appendChild(item_type::make_form_type(tr("Imagespace Modifier", disambig), dovah::form_type::imagespace_modifier))
            .appendChild(item_type::make_form_type(tr("Impact Data", disambig), dovah::form_type::impact_data))
            .appendChild(item_type::make_form_type(tr("Impact Data Set", disambig), dovah::form_type::impact_data_set))
            .appendChild(item_type::make_form_type(tr("Lens Flare", disambig), dovah::form_type::lens_flare))
            .appendChild(item_type::make_form_type(tr("Material Type", disambig), dovah::form_type::material_type))
            .appendChild(item_type::make_form_type(tr("Projectile", disambig), dovah::form_type::projectile))
            .appendChild(item_type::make_form_type(tr("Volumetric Lighting", disambig), dovah::form_type::volumetric_lighting))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("World Data", disambig))
            .appendChild(item_type::make_form_type(tr("Climate", disambig), dovah::form_type::climate))
            .appendChild(item_type::make_form_type(tr("Encounter Zone", disambig), dovah::form_type::encounter_zone))
            .appendChild(item_type::make_form_type(tr("Lighting Template", disambig), dovah::form_type::lighting_template))
            .appendChild(item_type::make_form_type(tr("Location", disambig), dovah::form_type::location))
            .appendChild(item_type::make_form_type(tr("Location Ref Type", disambig), dovah::form_type::location_ref_type))
            .appendChild(item_type::make_form_type(tr("Shader Particle Geometry", disambig), dovah::form_type::shader_particle_geometry_data))
            .appendChild(item_type::make_form_type(tr("Visual Effect", disambig), dovah::form_type::reference_effect))
            .appendChild(item_type::make_form_type(tr("Water Type", disambig), dovah::form_type::water_type))
            .appendChild(item_type::make_form_type(tr("Weather", disambig), dovah::form_type::weather))
            .appendChild(item_type::make_form_type(tr("Worldspace", disambig), dovah::form_type::worldspace))
      );
      this->_nodes.root->appendChild(
         item_type::make_top_level(tr("World Objects", disambig))
            .appendChild(item_type::make_form_type(tr("Activator", disambig), dovah::form_type::activator))
            .appendChild(item_type::make_form_type(tr("Container", disambig), dovah::form_type::container))
            .appendChild(item_type::make_form_type(tr("Door", disambig), dovah::form_type::door))
            .appendChild(item_type::make_form_type(tr("Flora", disambig), dovah::form_type::flora))
            .appendChild(item_type::make_form_type(tr("Furniture", disambig), dovah::form_type::furniture))
            .appendChild(item_type::make_form_type(tr("Grass", disambig), dovah::form_type::grass))
            .appendChild(item_type::make_form_type(tr("Light", disambig), dovah::form_type::light))
            .appendChild(item_type::make_form_type(tr("MovableStatic", disambig), dovah::form_type::movable_static))
            .appendChild(item_type::make_form_type(tr("Static", disambig), dovah::form_type::statik))
            .appendChild(item_type::make_form_type(tr("Static Collection", disambig), dovah::form_type::static_collection))
            .appendChild(item_type::make_form_type(tr("Tree", disambig), dovah::form_type::tree))
      );
      this->_nodes.root->appendChild(*this->_nodes.all);
      this->_nodes.root->appendChild(item_type::make_form_type(tr("Missing", disambig), dovah::form_type::none));
      #pragma endregion
      this->endResetModel();
      //
      auto& cache = dovahkit::subsystems::form_info_cache::core::get_or_create();
      QObject::connect(&cache, &dovahkit::subsystems::form_info_cache::core::cachedDataBuilt,   this, [this]() {
         this->_buildQuestFilters();
         this->_buildAllModelPathFilters();
      });
      QObject::connect(&cache, &dovahkit::subsystems::form_info_cache::core::cachedQuestFilterChanged, this, [this](const dovah::form_stub& stub, const QString prior, const QString after) {
         if (stub.form_type != dovah::form_type::quest)
            return;
         auto* root = this->_nodes.quests;
         if (!prior.isEmpty())
            this->_removeFilter(root, prior);
         if (!after.isEmpty())
            this->_addFilter(root, after, true);
      });
      QObject::connect(&cache, &dovahkit::subsystems::form_info_cache::core::cachedModelPathChanged, this, [this](const dovah::form_stub& stub, const QString prior, const QString after) {
         auto* root = this->_findFormTypeItem(stub.form_type);
         if (root) {
            if (!prior.isEmpty())
               this->_removeFilter(root, prior);
            if (after.isEmpty())
               this->_addFilter(root, after, true);
         }
      });
      QObject::connect(&cache, &dovahkit::subsystems::form_info_cache::core::cachedDataCleared, this, [this]() {
         this->_clearFilters(this->_nodes.quests);
         {
            constexpr const auto& form_types_of_interest = dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_types_of_interest;
            for (auto ft : form_types_of_interest) {
               auto* node = this->_findFormTypeItem(ft);
               if (node)
                  this->_clearFilters(node);
            }
         }
      });

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &ObjectWindowTreeModel::_onGameMaybeChanged);
      QObject::connect(&editor, &DovahKitCore::dataAbandonComplete, this, &ObjectWindowTreeModel::_onGameMaybeChanged);
      QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, &ObjectWindowTreeModel::_onGameMaybeChanged);
   }
   ObjectWindowTreeModel::~ObjectWindowTreeModel() {
      this->_nodes.all    = nullptr;
      this->_nodes.quests = nullptr;
      delete this->_nodes.root;
      this->_nodes.root   = nullptr;
   }

   ObjectWindowTreeModel::item_type* ObjectWindowTreeModel::_itemFromIndex(const QModelIndex& index) noexcept {
      if (!index.isValid())
         return nullptr;
      return (item_type*) index.internalPointer();
   }
   ObjectWindowTreeModel::item_type* ObjectWindowTreeModel::_findFormTypeItem(dovah::form_type form_type) const noexcept {
      return this->_nodes.root->findChildByFormType(form_type);
   }
   QModelIndex ObjectWindowTreeModel::_indexOfItem(item_type* item) const noexcept {
      if (!item)
         return QModelIndex();
      auto* parent = item->parent;
      if (!parent)
         return QModelIndex();
      return this->createIndex(parent->indexOf(item), 0, item);
   }
   QModelIndex ObjectWindowTreeModel::_indexOfQuests() const noexcept {
      return this->_indexOfItem(this->_nodes.quests);
   }
   QModelIndex ObjectWindowTreeModel::_indexOfAll() const noexcept {
      return this->_indexOfItem(this->_nodes.all);
   }
   bool ObjectWindowTreeModel::_removeRows(int row, int count, const QModelIndex& parent) {
      if (count < 1)
         return false;
      auto* item = _itemFromIndex(parent);
      if (!item)
         return false;
      auto& list = item->children;
      int   max  = row + count;
      if (max >= list.size())
         return false;
      this->beginRemoveRows(parent, row, max - 1);
      for (int i = row; i < max; ++i) {
         assert(list[i] && "ObjectWindowTreeItem::children should never contain null pointers.");
         delete list[i];
      }
      list.erase(list.begin() + row, list.begin() + max);
      this->endRemoveRows();
      return true;
   }
   void ObjectWindowTreeModel::_sortChildrenOf(item_type* item) {
      if (!item->children.size())
         return;
      //
      auto& list = item->children;
      auto  size = list.size();
      decltype(item->children) sorted;
      sorted.reserve(size);
      for (auto* child : list)
         sorted.push_back(child);
      std::sort(sorted.begin(), sorted.end(), [](const item_type* a, const item_type* b) {
         return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
      });
      //
      auto persistent = this->persistentIndexList();
      if (!persistent.isEmpty()) {
         QModelIndexList change_from;
         QModelIndexList change_to;
         for (int i = 0; i < size; ++i) {
            auto* item = list[i];
            auto  from = this->createIndex(i, 0, item);
            if (persistent.contains(from)) {
               auto to_i = sorted.indexOf(item);
               auto to   = this->createIndex(to_i, 0, item);
               change_from.push_back(from);
               change_to.push_back(to);
            }
         }
         if (!change_from.isEmpty()) {
            this->changePersistentIndexList(change_from, change_to);
         }
      }
      std::swap(list, sorted);
   }
   void ObjectWindowTreeModel::_sortDescendantsOf(item_type* item) {
      this->_sortChildrenOf(item);
      for (auto* child : item->children)
         this->_sortDescendantsOf(child);
   }

   #pragma region QAbstractItemModel overrides
      QModelIndex ObjectWindowTreeModel::index(int row, int column, const QModelIndex& parent) const {
         if (!this->hasIndex(row, column, parent))
            return QModelIndex();
         item_type* parentItem;
         if (!parent.isValid())
            parentItem = this->_nodes.root;
         else
            parentItem = static_cast<item_type*>(parent.internalPointer());
         item_type* childItem = parentItem->child(row);
         if (childItem)
            return this->createIndex(row, column, childItem);
         return QModelIndex();
      }
      QModelIndex ObjectWindowTreeModel::parent(const QModelIndex& index) const {
         if (auto* child = _itemFromIndex(index)) {
            if (auto* parent = child->parent) {
               if (parent != this->_nodes.root) {
                  assert(parent->parent);
                  auto i = parent->parent->indexOf(parent);
                  return this->createIndex(i, 0, parent);
               }
            }
         }
         return QModelIndex();
      }
      int ObjectWindowTreeModel::rowCount(const QModelIndex& parent) const {
         auto* item = _itemFromIndex(parent);
         if (!item)
            item = this->_nodes.root;
         return item->children.size();
      }
      int ObjectWindowTreeModel::columnCount(const QModelIndex& item) const {
         return 1;
      }
      Qt::ItemFlags ObjectWindowTreeModel::flags(const QModelIndex& index) const {
         if (!index.isValid())
            return Qt::NoItemFlags;
         auto* item = _itemFromIndex(index);
         if (item && item != this->_nodes.all) {
            //
            // Disable the treeview items for SSE forms, when in LE mode.
            //
            auto ft = item->form_type;
            if (ft.has_value()) {
               auto& info = dovah::form_type_info::lookup(ft.value());
               if (info.flags & dovah::form_type_info::flag::is_skyrim_special) {
                  if (DovahKitCore::get().get_current_game() == dovah::game::skyrim_classic) {
                     return Qt::NoItemFlags;
                  }
               }
            }
         }
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      }
      QVariant ObjectWindowTreeModel::data(const QModelIndex& index, int role) const {
         if (!index.isValid() || index.column() != 0)
            return QVariant();
         auto* item = static_cast<item_type*>(index.internalPointer());
         switch (role) {
            case Qt::DisplayRole:
               return item->name;
            case Qt::FontRole:
               if (item->type == item_type::type_t::filter)
                  return QVariant();
               {
                  QFont font;
                  font.setBold(true);
                  return font;
               }
         }
         return QVariant();
      }
      QVariant ObjectWindowTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
         return QVariant();
      }
   #pragma endregion

   QModelIndex ObjectWindowTreeModel::indexOfAllCategory() const noexcept {
      return this->_indexOfItem(this->_nodes.all);
   }
   QVector<dovah::form_type> ObjectWindowTreeModel::formTypesFor(const QModelIndexList& qmil) const noexcept {
      QVector<dovah::form_type> out;
      for (auto* child : this->_nodes.root->children)
         if (child->form_type != dovah::form_type::none)
            child->gatherFormTypes(out);
      return out;
   }
   ui::object_window::filter_info ObjectWindowTreeModel::getFilterInfoFor(const QModelIndexList& qmil) const noexcept {
      ui::object_window::filter_info out;
      if (qmil.contains(this->_indexOfItem(this->_nodes.all))) {
         //
         // The "All" item is selected.
         //
         for (auto* child : this->_nodes.root->children)
            if (child->form_type != dovah::form_type::none)
               child->gatherFormTypes(out.form_types);
         return out;
      }
      for (const auto& qmi : qmil) {
         auto* item = this->_itemFromIndex(qmi);
         if (!item)
            continue;
         item->gatherFormTypes(out.form_types);
         //
         // Filters:
         //
         if (item->type == item_type::type_t::filter) {
            auto ft = item->containingFormType();
            if (ft == dovah::form_type::quest) {
               out.filters.quest_filter_prefix = item->full_filter;
            } else if (dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_type_is_of_interest(ft)) {
               out.filters.model_path_prefix = item->full_filter;
            } else {
               //
               // No applicable filter.
               //
               continue;
            }
         }
      }
      return out;
   }

   void ObjectWindowTreeModel::_buildQuestFilters() {
      auto* root = this->_nodes.quests;
      auto& list = root->children;
      auto  qmi  = this->_indexOfItem(root);
      if (!list.isEmpty()) {
         this->beginRemoveRows(qmi, 0, list.size() - 1);
         root->clear();
         this->endRemoveRows();
      }
      //
      // We want to insert items into the root all at once, but the beginInsertRows function 
      // demands that we tell it how many rows we're inserting in advance. This means that 
      // we have to insert items into a "surrogate parent" instead of directly into the root; 
      // then, we can count the number of items, call beginInsertRows, transfer them into the 
      // root, and then proceed as planned.
      //
      auto* surrogate_parent = new item_type;

      auto _pathlike_string_to_nodes = [root, surrogate_parent](QString path) {
         constexpr const bool include_trailing = false;

         if (path.isEmpty())
            return;

         QString fragment;
         auto*   node = root;

         for (auto c : path) {
            if (c != '/' && c != '\\') {
               fragment += c;
               continue;
            }
            //
            // Handle path separators.
            //
            if (fragment.isEmpty()) { // Treat "Foo//Bar" the same as "Foo/Bar"
               continue;
            }
            auto* parent = (node == root) ? surrogate_parent : node;
            int   index  = parent->indexOf(fragment);
            if (index < 0) {
               //
               // This fragment doesn't exist, so create it.
               //
               auto* child = &item_type::make_filter(fragment);
               if (node == root) {
                  child->full_filter = fragment;
               } else {
                  child->full_filter = node->full_filter + '/' + fragment;
               }
               parent->appendChild(*child);
               node = child;
            } else {
               //
               // The fragment already exists. Let's just bump up its refcount.
               //
               node = parent->child(index);
            }
            ++node->refcount;
            fragment.clear();
         }
         if (include_trailing && !fragment.isEmpty()) {
            auto* parent = (node == root) ? surrogate_parent : node;
            int   index  = parent->indexOf(fragment);
            if (index < 0) {
               //
               // This fragment doesn't exist, so create it.
               //
               auto* child = &item_type::make_filter(fragment);
               if (node == root) {
                  child->full_filter = fragment;
               } else {
                  child->full_filter = node->full_filter + '/' + fragment;
               }
               parent->appendChild(*child);
               node = child;
            } else {
               //
               // The fragment already exists. Let's just bump up its refcount.
               //
               node = parent->child(index);
            }
            ++node->refcount;
         }
      };
      //
      auto& fic = dovahkit::subsystems::form_info_cache::core::get();
      fic.for_all_quest_filters(_pathlike_string_to_nodes);

      if (!surrogate_parent->children.isEmpty()) {
         this->beginInsertRows(qmi, 0, surrogate_parent->children.size() - 1);
         std::swap(list, surrogate_parent->children);
         delete surrogate_parent;
         for (auto* child : list)
            child->parent = root; // fix up parent/child relationships
         root->recursiveSort();
         this->endInsertRows();
      }
   }
   void ObjectWindowTreeModel::_buildAllModelPathFilters() {
      auto _pathlike_string_to_nodes = [](item_type* root, item_type* surrogate_parent, QString path) {
         constexpr const bool include_trailing = false;

         if (path.isEmpty())
            return;

         QString fragment;
         auto*   node = root;

         for (auto c : path) {
            if (c != '/' && c != '\\') {
               fragment += c;
               continue;
            }
            //
            // Handle path separators.
            //
            if (fragment.isEmpty()) { // Treat "Foo//Bar" the same as "Foo/Bar"
               continue;
            }
            auto* parent = (node == root) ? surrogate_parent : node;
            int   index  = parent->indexOf(fragment);
            if (index < 0) {
               //
               // This fragment doesn't exist, so create it.
               //
               auto* child = &item_type::make_filter(fragment);
               if (node == root) {
                  child->full_filter = fragment;
               } else {
                  child->full_filter = node->full_filter + '/' + fragment;
               }
               parent->appendChild(*child);
               node = child;
            } else {
               //
               // The fragment already exists. Let's just bump up its refcount.
               //
               node = parent->child(index);
            }
            ++node->refcount;
            fragment.clear();
         }
         if (include_trailing && !fragment.isEmpty()) {
            auto* parent = (node == root) ? surrogate_parent : node;
            int   index  = parent->indexOf(fragment);
            if (index < 0) {
               //
               // This fragment doesn't exist, so create it.
               //
               auto* child = &item_type::make_filter(fragment);
               if (node == root) {
                  child->full_filter = fragment;
               } else {
                  child->full_filter = node->full_filter + '/' + fragment;
               }
               parent->appendChild(*child);
               node = child;
            } else {
               //
               // The fragment already exists. Let's just bump up its refcount.
               //
               node = parent->child(index);
            }
            ++node->refcount;
         }
      };

      constexpr const auto& form_types_of_interest = dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_types_of_interest;

      constexpr const size_t form_type_count = form_types_of_interest.size();
      std::array<item_type*, form_type_count> roots = {};
      std::array<item_type*, form_type_count> surrogate_parents = {};
      for (size_t i = 0; i < form_type_count; ++i) {
         roots[i] = this->_findFormTypeItem(form_types_of_interest[i]);
         assert(roots[i]);
      }
      for (auto& ptr : surrogate_parents)
         ptr = new item_type;

      auto& fic = dovahkit::subsystems::form_info_cache::core::get();
      fic.for_all_form_model_paths([&roots, &surrogate_parents, &_pathlike_string_to_nodes](const dovah::form_stub& stub, QString path) {
         size_t i = 0;
         for (; i < form_types_of_interest.size(); ++i)
            if (stub.form_type == form_types_of_interest[i])
               break;
         if (i >= form_types_of_interest.size())
            return;

         _pathlike_string_to_nodes(roots[i], surrogate_parents[i], path);
      });

      for (size_t i = 0; i < form_type_count; ++i) {
         auto* root      = roots[i];
         auto* surrogate = surrogate_parents[i];

         auto& dst = root->children;
         auto  qmi = this->_indexOfItem(root);
         {
            if (!dst.isEmpty()) {
               this->beginRemoveRows(qmi, 0, dst.size() - 1);
               root->clear();
               this->endRemoveRows();
            }
         }
         if (!surrogate->children.isEmpty()) {
            this->beginInsertRows(qmi, 0, surrogate->children.size() - 1);
            std::swap(dst, surrogate->children);
            delete surrogate;
            for (auto* child : dst)
               child->parent = root; // fix up parent/child relationships
            root->recursiveSort();
            this->endInsertRows();
         }
      }
   }
   void ObjectWindowTreeModel::_clearFilters(item_type* root) {
      auto qmi = this->_indexOfItem(root);
      if (qmi.isValid())
         this->_removeRows(0, this->rowCount(qmi), qmi);
   }
   void ObjectWindowTreeModel::_removeFilter(item_type* root, const QString& full) {
      if (full.isEmpty())
         return;
      auto* node = root;
      {
         QString fragment;
         for (int i = 0; i < full.size(); ++i) {
            QChar c = full[i];
            if (c == '/' || c == '\\') {
               if (fragment.isEmpty()) // Treat "Foo//Bar" the same as "Foo/Bar"
                  continue;
               auto index = node->indexOf(fragment);
               fragment.clear();
               if (index < 0) {
                  node = nullptr;
                  break;
               }
               node = node->child(index);
               if (--node->refcount == 0)
                  break;
               continue;
            }
            fragment += c;
         }
         if (!fragment.isEmpty() && node) { // trailing fragment doesn't end in a slash
            auto index = node->indexOf(fragment);
            if (index < 0) {
               node = nullptr;
            } else {
               node = node->child(index);
               --node->refcount;
            }
         }
      }
      if (node && node->type == ObjectWindowTreeItem::type_t::filter && node->refcount == 0) {
         //
         // Destroy the outermost node whose refcount dropped to zero.
         //
         auto* parent = node->parent;
         auto  index  = parent->indexOf(node);
         this->_removeRows(index, 1, this->_indexOfItem(parent));
      }
   }
   void ObjectWindowTreeModel::_addFilter(item_type* root, const QString& filter, bool include_trailing) {
      QVector<item_type*> added_to;
      QString fragment;
      auto*   node = root;
      for (int i = 0; i < filter.size(); ++i) {
         QChar c = filter[i];
         if (c == '/' || c == '\\') {
            if (fragment.isEmpty()) // Treat "Foo//Bar" the same as "Foo/Bar"
               continue;
            auto index = node->indexOf(fragment);
            if (index < 0) {
               added_to.push_back(node);
               //
               auto  size = node->children.size();
               this->beginInsertRows(this->_indexOfItem(node), size, size);
               auto* child = &item_type::make_filter(fragment);
               if (node == root) {
                  child->full_filter = fragment;
               } else {
                  child->full_filter = node->full_filter + '/' + fragment;
               }
               node->appendChild(*child);
               this->endInsertRows();
               //
               node = child;
            } else {
               node = node->child(index);
            }
            ++node->refcount;
            fragment.clear();
            continue;
         }
         fragment += c;
      }
      if (include_trailing && !fragment.isEmpty()) {
         int   index  = node->indexOf(fragment);
         if (index < 0) {
            added_to.push_back(node);
            //
            // This fragment doesn't exist, so create it.
            //
            auto  size = node->children.size();
            this->beginInsertRows(this->_indexOfItem(node), size, size);
            auto* child = &item_type::make_filter(fragment);
            if (node == root) {
               child->full_filter = fragment;
            } else {
               child->full_filter = node->full_filter + '/' + fragment;
            }
            node->appendChild(*child);
            this->endInsertRows();
            //
            node = child;
         } else {
            //
            // The fragment already exists. Let's just bump up its refcount.
            //
            node = node->child(index);
         }
         ++node->refcount;
      }
      for (auto* node : added_to) {
         emit this->layoutAboutToBeChanged({ this->_indexOfItem(node) }, LayoutChangeHint::VerticalSortHint);
         this->_sortChildrenOf(node);
         emit this->layoutChanged({ this->_indexOfItem(node) }, LayoutChangeHint::VerticalSortHint);
      }
   }
   void ObjectWindowTreeModel::_onGameMaybeChanged() {
      auto& editor    = DovahKitCore::get();
      bool  allow_sse = true;
      if (editor.has_data()) {
         allow_sse = editor.get_current_game() != dovah::game::skyrim_classic;
      }

      constexpr const auto sse_form_types = []() {
         constexpr const auto count = []() -> size_t {
            size_t i = 0;
            for (const auto& info : dovah::form_types)
               if (info.flags & dovah::form_type_info::flag::is_skyrim_special)
                  ++i;
            return i;
         }();
         std::array<dovah::form_type, count> values = {};
         size_t i = 0;
         for (const auto& info : dovah::form_types)
            if (info.flags & dovah::form_type_info::flag::is_skyrim_special)
               values[i++] = info.form_type;
         return values;
      }();

      for (const auto ft : sse_form_types) {
         if (auto* item = _findFormTypeItem(ft)) {
            auto qmi = _indexOfItem(item);
            emit dataChanged(qmi, qmi);
         }
      }
   }
#pragma endregion

#pragma region ObjectWindowTree
ObjectWindowTree::ObjectWindowTree(QWidget* parent) : QLinedTreeView(parent) {
   auto* model = new model_type(this);
   this->setModel(model);
   //
   if (auto* sel = this->selectionModel()) {
      sel->select(model->indexOfAllCategory(), QItemSelectionModel::ClearAndSelect);
   }
   this->expandAll();
}
QVector<dovah::form_type> ObjectWindowTree::allPrimaryFormTypes() const noexcept {
   auto* model = (model_type*) this->model();
   auto list = model->formTypesFor({ model->indexOfAllCategory() });
   list.push_back(dovah::form_type::none); // don't forget the "Missing" category, which is outside of "All"!
   return list;
}
ui::object_window::filter_info ObjectWindowTree::filterInfo() const noexcept {
   auto* sm    = this->selectionModel();
   auto* model = (model_type*) this->model();
   if (!sm || !model)
      return {};
   return model->getFilterInfoFor(sm->selectedRows());
}
#pragma endregion