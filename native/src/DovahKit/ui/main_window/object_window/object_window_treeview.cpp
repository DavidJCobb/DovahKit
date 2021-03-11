#include "object_window_treeview.h"
#include "../../../dovah/form_stub_addenda.h"
#include "../../../editor/core.h"

#pragma region ObjectWindowTreeItem
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

void ObjectWindowTreeItem::gatherFormTypes(QVector<dovah::form_type_t>& out) const noexcept {
   if (this->form_type != no_form_type_filter) {
      if (!out.contains(this->form_type))
         out.push_back(this->form_type);
      return;
   }
   for (auto* child : this->children)
      if (child)
         child->gatherFormTypes(out);
}
#pragma endregion

#pragma region ObjectWindowTreeModel
   ObjectWindowTreeModel::ObjectWindowTreeModel(QObject* parent) : QAbstractItemModel(parent) {
      this->root = new item_type;
      this->root->type = item_type::item_type::root;
      //
      this->beginResetModel();
      #pragma region Build contents
      constexpr char* disambig = "object window";
      this->root->appendChild(
         item_type::make_top_level(tr("Actors", disambig))
            .appendChild(item_type::make_form_type(tr("ActorBase", disambig), dovah::form_type::actor_base))
            .appendChild(item_type::make_form_type(tr("Actor Action", disambig), dovah::form_type::action))
            .appendChild(item_type::make_form_type(tr("BodyPartData", disambig), dovah::form_type::body_part_data))
            .appendChild(item_type::make_form_type(tr("LeveledCharacter", disambig), dovah::form_type::leveled_character))
            .appendChild(item_type::make_form_type(tr("Perk", disambig), dovah::form_type::perk))
            .appendChild(item_type::make_form_type(tr("TalkingActivator", disambig), dovah::form_type::talking_activator))
      );
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(
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
      this->root->appendChild(item_type::make_form_type(tr("All", disambig),     item_type::no_form_type_filter));
      this->root->appendChild(item_type::make_form_type(tr("Missing", disambig), dovah::form_type::none));
      #pragma endregion
      this->endResetModel();
      //
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ObjectWindowTreeModel::clearAllQuestFilters);
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &ObjectWindowTreeModel::buildAllQuestFilters);
      QObject::connect(&editor, &DovahKitCore::formModificationImminent, this, [this](dovah::form_stub* form) { // TODO: this will break if we receive imminents for multiple forms before any corresponding commits
         if (form->formType != dovah::form_type::quest)
            return;
         QString filter;
         if (form->addenda)
            filter = QString::fromStdString(form->addenda->filter);
         this->prepForQuestFilterChange(filter);
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
         if (form->formType != dovah::form_type::quest)
            return;
         QString filter;
         if (form->addenda)
            filter = QString::fromStdString(form->addenda->filter);
         this->finishQuestFilterChange(filter);
      });
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
      return this->_indexOfItem(this->quests);
   }
   QModelIndex ObjectWindowTreeModel::_indexOfAll() const noexcept {
      return this->_indexOfItem(this->all);
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
   }

   #pragma region QAbstractItemModel overrides
      QModelIndex ObjectWindowTreeModel::index(int row, int column, const QModelIndex& parent) const {
         if (!this->hasIndex(row, column, parent))
            return QModelIndex();
         item_type* parentItem;
         if (!parent.isValid())
            parentItem = this->root;
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
               if (parent != this->root) {
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
            item = this->root;
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

   void ObjectWindowTreeModel::buildAllQuestFilters() {
      this->clearAllQuestFilters();
      auto qmi = this->_indexOfQuests();
      if (!qmi.isValid())
         return;
      //
      struct _working {
         QString text;
         int     refcount = 0;
      };
      QHash<QString, _working> hash;
      DovahKitCore::get().for_each_form_of_type(dovah::form_type::quest, [&hash](dovah::form_stub* stub) {
         if (!stub->addenda)
            return false; // continue
         auto filter = QString::fromStdString(stub->addenda->filter);
         if (!filter.isEmpty()) {
            auto& entry = hash[filter.toLower()];
            if (entry.text.isEmpty())
               entry.text = filter;
            ++entry.refcount;
         }
         return false; // continue
      });
      //
      this->beginInsertRows(qmi, 0, hash.size());
      auto* parent = _itemFromIndex(qmi);
      auto& list   = parent->children;
      for (auto& entry : hash) {
         auto& item = item_type::make_filter(entry.text);
         item.refcount = entry.refcount;
         list.push_back(&item);
      }
      hash.clear();
      qSort(list.begin(), list.end(), [](const item_type* a, const item_type* b) {
         return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
      });
      this->endInsertRows();
   }
   void ObjectWindowTreeModel::clearAllQuestFilters() {
      auto qmi = this->_indexOfQuests();
      if (qmi.isValid())
         this->_removeRows(0, this->rowCount(qmi), qmi);
   }
   void ObjectWindowTreeModel::prepForQuestFilterChange(const QString& filter) {
      if (filter.isEmpty())
         return;
      auto qmi = this->_indexOfQuests();
      if (!qmi.isValid())
         return;
      auto* item = _itemFromIndex(qmi);
      for (auto* child : item->children) {
         auto& name = child->name;
         if (filter.compare(name, Qt::CaseInsensitive) == 0) {
            --child->refcount;
            break;
         }
      }
   }
   void ObjectWindowTreeModel::finishQuestFilterChange(const QString& filter) {
      auto qmi = this->_indexOfQuests();
      if (!qmi.isValid())
         return;
      auto* item  = _itemFromIndex(qmi);
      auto& list  = item->children;
      auto  size  = list.size();
      bool  found = false;
      for (int i = 0; i < size; ++i) {
         auto* child = list[i];
         if (!found && filter.compare(child->name, Qt::CaseInsensitive) == 0) {
            ++child->refcount;
            found = true;
         } else if (child->refcount == 0) {
            this->_removeRows(i, 1, qmi);
            --i;
            --size;
            continue;
         }
      }
      if (!found && !filter.isEmpty()) {
         //
         // We need to insert a new filter item. To avoid the overhead of re-sorting the entire 
         // list, let's just find the right place to insert this item. After all, we should have 
         // sorted the list when we initially generate it, so as long as we never do anything to 
         // break that sorting, we oughta be fine.
         //
         int i = 0;
         for (; i < size; ++i) {
            auto* child = list[i];
            if (child->name.compare(filter, Qt::CaseInsensitive) > 0) {
               //
               // This is the first child that would go after the new filter.
               //
               break;
            }
         }
         this->beginInsertRows(qmi, i, i);
         auto& child = item_type::make_filter(filter);
         child.refcount = 1;
         list.insert(i, &child);
         this->endInsertRows();
      }
   }
#pragma endregion