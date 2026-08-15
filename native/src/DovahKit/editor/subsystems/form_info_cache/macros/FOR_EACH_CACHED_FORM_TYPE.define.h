// Pass your macro name as an argument. The macro params should resemble:
//    #define DO(name, signal_name, ...)
// The macro should be variadic so we can add extra info without your macro choking.
#define FOR_EACH_CACHED_FORM_TYPE(DO) \
   DO(actor_base,      cachedActorBaseChanged) \
   DO(collision_layer, cachedCollisionLayerChanged) \
   DO(enchantment,     cachedEnchantmentChanged) \
   DO(faction,         cachedFactionChanged) \
   DO(head_part,       cachedHeadPartChanged) \
   DO(magic_effect,    cachedMagicEffectChanged) \
   DO(music_track,     cachedMusicTrackChanged) \
   DO(quest,           cachedQuestChanged) \
   DO(package,         cachedPackageChanged) \
   DO(topic,           cachedTopicChanged) \
   DO(voicetype,       cachedVoicetypeChanged)