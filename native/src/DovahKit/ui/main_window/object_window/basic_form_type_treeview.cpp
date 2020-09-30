#include "basic_form_type_treeview.h"

#pragma region BasicFormTypeTreeModelItem
void BasicFormTypeTreeModelItem::_destroyDescendants() noexcept {
   for (auto& child : this->_children) {
      child->_destroyDescendants();
      delete child;
      child = nullptr;
   }
   this->_children.clear();
}
void BasicFormTypeTreeModelItem::appendChild(BasicFormTypeTreeModelItem* child) noexcept {
   auto p = child->parent();
   if (p)
      p->removeChild(child);
   this->_children.push_back(child);
   child->_parent = this;
}
int32_t BasicFormTypeTreeModelItem::indexOf(BasicFormTypeTreeModelItem* child) const noexcept {
   for (size_t i = 0; i < this->_children.size(); i++)
      if (this->_children[i] == child)
         return i;
   return -1;
}
void BasicFormTypeTreeModelItem::removeChild(BasicFormTypeTreeModelItem* child) noexcept {
   auto& list = this->_children;
   list.erase(std::remove(list.begin(), list.end(), child), list.end());
   child->_parent = nullptr;
}

void BasicFormTypeTreeModelItem::addToSet(QVector<dovah::form_type_t>& out) const noexcept {
   if (this->form_type != dovah::form_type::none) {
      if (!out.contains(this->form_type))
         out.push_back(this->form_type);
      return;
   }
   for (auto* child : this->children())
      if (child)
         child->addToSet(out);
}
#pragma endregion

#pragma region BasicFormTypeTreeModel
QModelIndex BasicFormTypeTreeModel::index(int row, int column, const QModelIndex& parent) const {
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
QModelIndex BasicFormTypeTreeModel::parent(const QModelIndex& index) const {
   if (!index.isValid())
      return QModelIndex();
   item_type* childItem = static_cast<item_type*>(index.internalPointer());
   item_type* parentItem = childItem->parent();
   if (parentItem == this->root)
      return QModelIndex();
   auto i = parentItem->parent()->indexOf(parentItem);
   return createIndex(i, 0, parentItem);
}
int BasicFormTypeTreeModel::rowCount(const QModelIndex& parent) const {
   item_type* parentItem;
   if (parent.column() > 0)
      return 0;
   if (!parent.isValid())
      parentItem = this->root;
   else
      parentItem = static_cast<item_type*>(parent.internalPointer());
   return parentItem->childCount();
}
int BasicFormTypeTreeModel::columnCount(const QModelIndex& item) const {
   return 1;
}
Qt::ItemFlags BasicFormTypeTreeModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant BasicFormTypeTreeModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item = static_cast<item_type*>(index.internalPointer());
   if (role != Qt::DisplayRole)
      return QVariant();
   if (index.column() == 0)
      return item->name();
   return QVariant();
}
//
QVariant BasicFormTypeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
   return QVariant();
}
//
void BasicFormTypeTreeModel::clear() {
   this->beginResetModel();
   this->root->_destroyDescendants();
   this->endResetModel();
}
#pragma endregion

#pragma region BasicFormTypeTree
BasicFormTypeTree::BasicFormTypeTree(QWidget* parent) : QLinedTreeView(parent) {
   this->setModel(new model_type);
   //
   constexpr char* disambig = "object window";
   auto model = static_cast<model_type*>(this->model());
   {
      auto parent = new model_item_type(tr("Actors", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("ActorBase", disambig), dovah::form_type::actor_base));
      parent->appendChild(new model_item_type(tr("Actor Action", disambig), dovah::form_type::action));
      parent->appendChild(new model_item_type(tr("BodyPartData", disambig), dovah::form_type::body_part_data));
      parent->appendChild(new model_item_type(tr("LeveledCharacter", disambig), dovah::form_type::leveled_character));
      parent->appendChild(new model_item_type(tr("Perk", disambig), dovah::form_type::perk));
      parent->appendChild(new model_item_type(tr("TalkingActivator", disambig), dovah::form_type::talking_activator));
   }
   {
      auto parent = new model_item_type(tr("Audio", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Acoustic Space", disambig), dovah::form_type::acoustic_space));
      parent->appendChild(new model_item_type(tr("Music Track", disambig), dovah::form_type::music_track));
      parent->appendChild(new model_item_type(tr("Music Type", disambig), dovah::form_type::music_type));
      parent->appendChild(new model_item_type(tr("Reverb Parameters", disambig), dovah::form_type::reverb_parameters));
      parent->appendChild(new model_item_type(tr("Sound Category", disambig), dovah::form_type::sound_category));
      parent->appendChild(new model_item_type(tr("Sound Description", disambig), dovah::form_type::sound_descriptor));
      parent->appendChild(new model_item_type(tr("Sound Emitter", disambig), dovah::form_type::sound));
      parent->appendChild(new model_item_type(tr("Sound Output Model", disambig), dovah::form_type::sound_output_model));
   }
   {
      auto parent = new model_item_type(tr("Character", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Association Type", disambig), dovah::form_type::association_type));
      parent->appendChild(new model_item_type(tr("Class", disambig), dovah::form_type::combat_class));
      parent->appendChild(new model_item_type(tr("Equip Slot", disambig), dovah::form_type::equip_slot));
      parent->appendChild(new model_item_type(tr("Faction", disambig), dovah::form_type::faction));
      parent->appendChild(new model_item_type(tr("HeadPart", disambig), dovah::form_type::head_part));
      parent->appendChild(new model_item_type(tr("Movement Type", disambig), dovah::form_type::movement_type));
      parent->appendChild(new model_item_type(tr("Package", disambig), dovah::form_type::package));
      parent->appendChild(new model_item_type(tr("Quest", disambig), dovah::form_type::quest));
      parent->appendChild(new model_item_type(tr("Race", disambig), dovah::form_type::race));
      parent->appendChild(new model_item_type(tr("Relationship", disambig), dovah::form_type::relationship));
      parent->appendChild(new model_item_type(tr("SM Event Node", disambig), dovah::form_type::story_event_node));
      parent->appendChild(new model_item_type(tr("VoiceType", disambig), dovah::form_type::voicetype));
   }
   {
      auto parent = new model_item_type(tr("Items", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Ammo", disambig), dovah::form_type::ammo));
      parent->appendChild(new model_item_type(tr("Armor", disambig), dovah::form_type::armor));
      parent->appendChild(new model_item_type(tr("ArmorAddon", disambig), dovah::form_type::armor_addon));
      parent->appendChild(new model_item_type(tr("Book", disambig), dovah::form_type::book));
      parent->appendChild(new model_item_type(tr("Constructible Object", disambig), dovah::form_type::constructible_object));
      parent->appendChild(new model_item_type(tr("Ingredient", disambig), dovah::form_type::ingredient));
      parent->appendChild(new model_item_type(tr("Key", disambig), dovah::form_type::key));
      parent->appendChild(new model_item_type(tr("LeveledItem", disambig), dovah::form_type::leveled_item));
      parent->appendChild(new model_item_type(tr("MiscItem", disambig), dovah::form_type::misc_item));
      parent->appendChild(new model_item_type(tr("Outfit", disambig), dovah::form_type::outfit));
      parent->appendChild(new model_item_type(tr("Soul Gem", disambig), dovah::form_type::soul_gem));
      parent->appendChild(new model_item_type(tr("Weapon", disambig), dovah::form_type::weapon));
   }
   {
      auto parent = new model_item_type(tr("Magic", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Dual Cast Data", disambig), dovah::form_type::dual_cast_data));
      parent->appendChild(new model_item_type(tr("Enchantment", disambig), dovah::form_type::enchantment));
      parent->appendChild(new model_item_type(tr("LeveledSpell", disambig), dovah::form_type::leveled_spell));
      parent->appendChild(new model_item_type(tr("Magic Effect", disambig), dovah::form_type::magic_effect));
      parent->appendChild(new model_item_type(tr("Potion", disambig), dovah::form_type::potion));
      parent->appendChild(new model_item_type(tr("Scroll", disambig), dovah::form_type::scroll));
      parent->appendChild(new model_item_type(tr("Shout", disambig), dovah::form_type::shout));
      parent->appendChild(new model_item_type(tr("Spell", disambig), dovah::form_type::spell));
      parent->appendChild(new model_item_type(tr("Word of Power", disambig), dovah::form_type::word_of_power));
   }
   {
      auto parent = new model_item_type(tr("Miscellaneous", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Animation Prop", disambig), dovah::form_type::animation_prop));
      parent->appendChild(new model_item_type(tr("Art Object", disambig), dovah::form_type::art_object));
      parent->appendChild(new model_item_type(tr("Collision Layer", disambig), dovah::form_type::collision_layer));
      parent->appendChild(new model_item_type(tr("Color", disambig), dovah::form_type::color));
      parent->appendChild(new model_item_type(tr("Combat Style", disambig), dovah::form_type::combat_style));
      parent->appendChild(new model_item_type(tr("FormList", disambig), dovah::form_type::formlist));
      parent->appendChild(new model_item_type(tr("Global", disambig), dovah::form_type::global));
      parent->appendChild(new model_item_type(tr("Idle Marker", disambig), dovah::form_type::idle_marker));
      parent->appendChild(new model_item_type(tr("Keyword", disambig), dovah::form_type::keyword));
      parent->appendChild(new model_item_type(tr("Land Texture", disambig), dovah::form_type::land_texture));
      parent->appendChild(new model_item_type(tr("Loading Screen", disambig), dovah::form_type::loading_screen));
      parent->appendChild(new model_item_type(tr("Material Object", disambig), dovah::form_type::material_object));
      parent->appendChild(new model_item_type(tr("Message", disambig), dovah::form_type::message));
      parent->appendChild(new model_item_type(tr("TextureSet", disambig), dovah::form_type::texture_set));
   }
   {
      auto parent = new model_item_type(tr("Special Effects", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Add-on Node", disambig), dovah::form_type::addon_node));
      parent->appendChild(new model_item_type(tr("Camera Shot", disambig), dovah::form_type::camera_shot));
      parent->appendChild(new model_item_type(tr("Debris", disambig), dovah::form_type::debris));
      parent->appendChild(new model_item_type(tr("EffectShader", disambig), dovah::form_type::effect_shader));
      parent->appendChild(new model_item_type(tr("Explosion", disambig), dovah::form_type::explosion));
      parent->appendChild(new model_item_type(tr("Footstep", disambig), dovah::form_type::footstep));
      parent->appendChild(new model_item_type(tr("Footstep Set", disambig), dovah::form_type::footstep_set));
      parent->appendChild(new model_item_type(tr("Hazard", disambig), dovah::form_type::hazard));
      parent->appendChild(new model_item_type(tr("Imagespace", disambig), dovah::form_type::imagespace));
      parent->appendChild(new model_item_type(tr("Imagespace Modifier", disambig), dovah::form_type::imagespace_modifier));
      parent->appendChild(new model_item_type(tr("Impact Data", disambig), dovah::form_type::impact_data));
      parent->appendChild(new model_item_type(tr("Impact Data Set", disambig), dovah::form_type::impact_data_set));
      parent->appendChild(new model_item_type(tr("Material Type", disambig), dovah::form_type::material_type));
      parent->appendChild(new model_item_type(tr("Projectile", disambig), dovah::form_type::projectile));
   }
   {
      auto parent = new model_item_type(tr("World Data", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Climate", disambig), dovah::form_type::climate));
      parent->appendChild(new model_item_type(tr("Encounter Zone", disambig), dovah::form_type::encounter_zone));
      parent->appendChild(new model_item_type(tr("Lighting Template", disambig), dovah::form_type::lighting_template));
      parent->appendChild(new model_item_type(tr("Location", disambig), dovah::form_type::location));
      parent->appendChild(new model_item_type(tr("Location Ref Type", disambig), dovah::form_type::location_ref_type));
      parent->appendChild(new model_item_type(tr("Shader Particle Geometry", disambig), dovah::form_type::shader_particle_geometry_data));
      parent->appendChild(new model_item_type(tr("Visual Effect", disambig), dovah::form_type::reference_effect));
      parent->appendChild(new model_item_type(tr("Water Type", disambig), dovah::form_type::water_type));
      parent->appendChild(new model_item_type(tr("Weather", disambig), dovah::form_type::weather));
   }
   {
      auto parent = new model_item_type(tr("World Objects", disambig), dovah::form_type::none);
      model->invisibleRootItem()->appendChild(parent);
      //
      parent->appendChild(new model_item_type(tr("Activator", disambig), dovah::form_type::activator));
      parent->appendChild(new model_item_type(tr("Container", disambig), dovah::form_type::container));
      parent->appendChild(new model_item_type(tr("Door", disambig), dovah::form_type::door));
      parent->appendChild(new model_item_type(tr("Flora", disambig), dovah::form_type::flora));
      parent->appendChild(new model_item_type(tr("Furniture", disambig), dovah::form_type::furniture));
      parent->appendChild(new model_item_type(tr("Grass", disambig), dovah::form_type::grass));
      parent->appendChild(new model_item_type(tr("Light", disambig), dovah::form_type::light));
      parent->appendChild(new model_item_type(tr("MovableStatic", disambig), dovah::form_type::movable_static));
      parent->appendChild(new model_item_type(tr("Static", disambig), dovah::form_type::statik));
      parent->appendChild(new model_item_type(tr("Static Collection", disambig), dovah::form_type::static_collection));
      parent->appendChild(new model_item_type(tr("Tree", disambig), dovah::form_type::tree));
   }
   model->invisibleRootItem()->appendChild(new model_item_type(tr("All", disambig), dovah::form_type::none));
   //
   this->expandAll();
}
QVector<dovah::form_type_t> BasicFormTypeTree::selectedFormTypes() const noexcept {
   QVector<dovah::form_type_t> out;
   auto sel = this->selectionModel();
   auto a   = sel->selection(); // using (sel->selectedRows()) will cause you to act on the previous selection, not the current one. naturally, this isn't bloody documented anywhere
   for (const auto& idx : a.indexes()) {
      const auto* item = (model_item_type*)idx.internalPointer();
      if (!item)
         continue;
      item->addToSet(out);
   }
   if (out.empty()) {
      auto model = (model_type*)this->model();
      auto root  = model->invisibleRootItem();
      for (auto child : root->children())
         child->addToSet(out);
   }
   return out;
}
#pragma endregion