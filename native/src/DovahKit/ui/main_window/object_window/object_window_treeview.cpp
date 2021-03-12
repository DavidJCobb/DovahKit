#include "object_window_treeview.h"
#include "../../../dovah/form_stub_addenda.h"
#include "../../../editor/core.h"

#pragma region ObjectWindowTreeItem
ObjectWindowTreeItem::~ObjectWindowTreeItem() {
   this->clear();
}

/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_filter(const QString& name) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = item_type::filter;
   return *item;
}
/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_top_level(const QString& name) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = item_type::top_level;
   return *item;
}
/*static*/ ObjectWindowTreeItem& ObjectWindowTreeItem::make_form_type(const QString& name, int ft) {
   auto* item = new ObjectWindowTreeItem;
   item->name = name;
   item->type = item_type::top_level;
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

void ObjectWindowTreeItem::clear() {
   for (auto* child : this->children)
      delete child;
   this->children.clear();
}
dovah::form_type_t ObjectWindowTreeItem::containingFormType() const noexcept {
   auto* node = this;
   do {
      auto ft = node->form_type;
      if (ft != dovah::form_type::none && ft != no_form_type_filter)
         return ft;
   } while (node = node->parent);
   return dovah::form_type::none;
}
void ObjectWindowTreeItem::gatherFormTypes(QVector<dovah::form_type_t>& out) const noexcept {
   if (this->form_type != no_form_type_filter) {
      if (!out.contains(this->form_type))
         out.push_back(this->form_type);
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
   qSort(list.begin(), list.end(), [](const ObjectWindowTreeItem* a, const ObjectWindowTreeItem* b) {
      return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
   });
}
#pragma endregion

#pragma region ObjectWindowFilterInfo
bool ObjectWindowFilterInfo::operator==(const ObjectWindowFilterInfo& other) const noexcept {
   if (this->filters.quests.size() != other.filters.quests.size())
      return false;
   if (this->form_types.size() != other.form_types.size())
      return false;
   for (auto ft : this->form_types)
      if (!other.form_types.contains(ft))
         return false;
   for (const auto& a : this->filters.quests)
      for (const auto& b : other.filters.quests)
         if (a.compare(b, Qt::CaseInsensitive) != 0)
            return false;
   return true;
}
ObjectWindowFilterInfo::filter_list_t* ObjectWindowFilterInfo::filterListFor(dovah::form_type_t ft) noexcept {
   switch (ft) {
      case dovah::form_type::quest:
         return &this->filters.quests;
   }
   return nullptr;
}
const ObjectWindowFilterInfo::filter_list_t* ObjectWindowFilterInfo::filterListFor(dovah::form_type_t ft) const noexcept {
   return const_cast<ObjectWindowFilterInfo*>(this)->filterListFor(ft);
}
bool ObjectWindowFilterInfo::testFormStubFilter(const dovah::form_stub* stub) const noexcept {
   auto* list = this->filterListFor(stub->formType);
   if (!list)
      return true;
   if (list->isEmpty())
      return true;
   //
   // The list is not empty, so apply the filters therein. Treat it as an "OR" match.
   //
   if (!stub->addenda)
      return false;
   auto filter = QString::fromStdString(stub->addenda->filter);
   for (const auto& match : *list)
      if (filter.compare(match, Qt::CaseInsensitive) == 0)
         return true;
   return false;
}
#pragma endregion

#pragma region ObjectWindowTreeModel
   ObjectWindowTreeModel::ObjectWindowTreeModel(QObject* parent) : QAbstractItemModel(parent) {
      this->_nodes.root = new item_type;
      this->_nodes.root->type = item_type::item_type::root;
      //
      this->beginResetModel();
      #pragma region Build contents
      constexpr char* disambig = "object window";
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
            .appendChild(item_type::make_form_type(tr("Sound Description", disambig), dovah::form_type::sound_descriptor))
            .appendChild(item_type::make_form_type(tr("Sound Emitter", disambig), dovah::form_type::sound))
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
            .appendChild(item_type::make_form_type(tr("Quest", disambig), dovah::form_type::quest))
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
            .appendChild(item_type::make_form_type(tr("Material Type", disambig), dovah::form_type::material_type))
            .appendChild(item_type::make_form_type(tr("Projectile", disambig), dovah::form_type::projectile))
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
      this->_nodes.root->appendChild(item_type::make_form_type(tr("All", disambig),     item_type::no_form_type_filter));
      this->_nodes.root->appendChild(item_type::make_form_type(tr("Missing", disambig), dovah::form_type::none));
      #pragma endregion
      this->endResetModel();
      //
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectWindowTreeModel::clearAllQuestFilters);
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &ObjectWindowTreeModel::buildAllQuestFilters);
      QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, [this](dovah::form_stub* form) { // TODO: this will break if we receive imminents for multiple forms before any corresponding commits
         if (form->formType != dovah::form_type::quest)
            return;
         if (form->addenda)
            this->_pending_filter_changes[form->formID] = QString::fromStdString(form->addenda->filter);
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
         if (form->formType != dovah::form_type::quest)
            return;
         QString filter;
         if (form->addenda)
            filter = QString::fromStdString(form->addenda->filter);
         this->finishQuestFilterChange(form->formID, filter);
      });
   }
   ObjectWindowTreeModel::~ObjectWindowTreeModel() {
      this->_nodes.all    = nullptr;
      this->_nodes.quests = nullptr;
      delete this->_nodes.root;
      this->_nodes.root   = nullptr;
      //
      this->_pending_filter_changes.clear();
   }

   ObjectWindowTreeModel::item_type* ObjectWindowTreeModel::_itemFromIndex(const QModelIndex& index) noexcept {
      if (!index.isValid())
         return nullptr;
      return (item_type*) index.internalPointer();
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
      qSort(sorted.begin(), sorted.end(), [](const item_type* a, const item_type* b) {
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
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
      }
      QVariant ObjectWindowTreeModel::data(const QModelIndex& index, int role) const {
         if (!index.isValid() || index.column() != 0)
            return QVariant();
         auto* item = static_cast<item_type*>(index.internalPointer());
         switch (role) {
            case Qt::DisplayRole:
               return item->name;
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
   QVector<dovah::form_type_t> ObjectWindowTreeModel::formTypesFor(const QModelIndexList& qmil) const noexcept {
      QVector<dovah::form_type_t> out;
      for (auto& qmi : qmil) {
         auto* item = this->_itemFromIndex(qmi);
         if (item) {
            item->gatherFormTypes(out);
         }
      }
      return out;
   }
   ObjectWindowFilterInfo ObjectWindowTreeModel::getFilterInfoFor(const QModelIndexList& qmil) const noexcept {
      ObjectWindowFilterInfo out;
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
         if (item->type != item_type::item_type::filter)
            continue;
         const auto& full = item->full_filter;
         if (full.isEmpty())
            continue;
         auto* gather_to = out.filterListFor(item->containingFormType());
         if (!gather_to)
            continue;
         bool found = false;
         for (auto& existing : *gather_to) {
            if (existing.startsWith(full, Qt::CaseInsensitive)) {
               existing = full;
               found    = true;
               break;
            }
         }
         if (!found)
            gather_to->push_back(full);
      }
      return out;
   }

   void ObjectWindowTreeModel::buildAllQuestFilters() {
      assert(this->_nodes.quests);
      //
      this->beginResetModel();
      //
      this->_nodes.quests->clear();
      //
      DovahKitCore::get().for_each_form_of_type(dovah::form_type::quest, [this](dovah::form_stub* stub) {
         if (!stub->addenda)
            return false; // continue
         auto filter = QString::fromStdString(stub->addenda->filter);
         if (filter.isEmpty())
            return false; // continue
         //
         // Increment refcounts and create nodes as appropriate:
         //
         QString fragment;
         auto*   node = this->_nodes.quests;
         for (int i = 0; i < filter.size(); ++i) {
            QChar c = filter[i];
            if (c == '/' || c == '\\') {
               if (fragment.isEmpty()) // Treat "Foo//Bar" the same as "Foo/Bar"
                  continue;
               auto index = node->indexOf(fragment);
               if (index < 0) {
                  auto* child = &item_type::make_filter(fragment);
                  if (node == this->_nodes.quests) {
                     child->full_filter = fragment;
                  } else {
                     child->full_filter = node->full_filter + '/' + fragment;
                  }
                  node->children.push_back(child);
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
         return false; // continue
      });
      //
      this->_nodes.quests->recursiveSort();
      //
      this->endResetModel();
   }
   void ObjectWindowTreeModel::clearAllQuestFilters() {
      auto qmi = this->_indexOfQuests();
      if (qmi.isValid())
         this->_removeRows(0, this->rowCount(qmi), qmi);
   }
   void ObjectWindowTreeModel::finishQuestFilterChange(dovah::bare_form_id_t formID, const QString& filter) {
      QString old = this->_pending_filter_changes.value(formID);
      this->_pending_filter_changes.remove(formID);
      if (old.compare(filter, Qt::CaseInsensitive) == 0) // no change
         return;
      //
      if (!old.isEmpty()) {
         //
         // Decrement refcounts as appropriate:
         //
         QString fragment;
         auto*   node = this->_nodes.quests;
         for (int i = 0; i < old.size(); ++i) {
            QChar c = old[i];
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
         if (node && node != this->_nodes.quests && node->refcount == 0) {
            //
            // Destroy the outermost node whose refcount dropped to zero.
            //
            auto* parent = node->parent;
            auto  index  = parent->indexOf(node);
            this->_removeRows(index, 0, this->_indexOfItem(parent));
         }
      }
      //
      // Increment refcounts and create nodes as appropriate:
      //
      QVector<item_type*> added_to;
      QString fragment;
      auto*   node = this->_nodes.quests;
      for (int i = 0; i < filter.size(); ++i) {
         QChar c = filter[i];
         if (c == '/' || c == '\\') {
            if (fragment.isEmpty()) // Treat "Foo//Bar" the same as "Foo/Bar"
               continue;
            auto index = node->indexOf(fragment);
            if (index < 0) {
               QString full_filter;
               if (node != this->_nodes.quests)
                  full_filter = node->full_filter + '/';
               full_filter += fragment;
               //
               added_to.push_back(node);
               auto& list = node->children;
               auto  size = list.size();
               this->beginInsertRows(this->_indexOfItem(node), size, size);
               node = &item_type::make_filter(fragment);
               node->full_filter = full_filter;
               list.push_back(node);
               this->endInsertRows();
            } else {
               node = node->child(index);
            }
            ++node->refcount;
            fragment.clear();
            continue;
         }
         fragment += c;
      }
      for (auto* node : added_to) {
         emit this->layoutAboutToBeChanged({ this->_indexOfItem(node) }, LayoutChangeHint::VerticalSortHint);
         this->_sortChildrenOf(node);
         emit this->layoutChanged({ this->_indexOfItem(node) }, LayoutChangeHint::VerticalSortHint);
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
ObjectWindowFilterInfo ObjectWindowTree::filterInfo() const noexcept {
   auto* sm    = this->selectionModel();
   auto* model = (model_type*) this->model();
   if (!sm || !model)
      return ObjectWindowFilterInfo();
   return model->getFilterInfoFor(sm->selectedRows());
}
#pragma endregion