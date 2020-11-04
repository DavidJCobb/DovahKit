#include "get_game_setting_description.h"
#include <array>
#include <QObject>

//
// These should be localized, so I don't want to embed them into the backend.
//

namespace {
   const char* disambig = "GMST description";

   struct _association {
      const char* name;
      QString     description;
      //
      _association() {}
      _association(const char* c, const QString& s) : name(c), description(s) {}
   };
   std::array _associations = {
      _association("fDetectionUpdateTimeMaxComplex", QObject::tr("This is used in place of fDetectionUpdateTimeMax if any attached cells contain ComplexSceneMarkers.", disambig)),
      _association("fDetectionUpdateTimeMinComplex", QObject::tr("This is used in place of fDetectionUpdateTimeMin if any attached cells contain ComplexSceneMarkers.", disambig)),
      //
      _association("fEssentialDeathTime",            QObject::tr("The number of seconds that an essential actor stays in bleedout once their health is drained.", disambig)),
      //
      _association("iInventoryAskQuantityAt",        QObject::tr("When dropping items from your inventory, you will be prompted to select a quantity if you have more than this many of an item.", disambig)),
      //
      _association("iMaxAttachedArrows",             QObject::tr("The maximum number of arrows or similar projectiles that can be stuck in a body. This is purely cosmetic.", disambig)),
      _association("iMaxCharacterLevel",             QObject::tr("Unused. In Oblivion, this setting determined the maximum level that the player could reach.", disambig)),
      _association("iMaxPlayerRunes",                QObject::tr("The maximum number of runes that the player can have active at a time.", disambig)),
      _association("iMaxSummonedCreatures",          QObject::tr("The maximum number of summoned creatures that the player can have at a time.", disambig)),
      //
      _association("sActivationChoiceMessage",   QObject::tr("The text of a message box menu displayed when the player activates an object, if the player's perks result in them having multiple activation choices for the object.", disambig)),
      _association("sAmber",                     QObject::tr("Unused. A color option for Fallout's Pip-Boy and associated UI.", disambig)),
      _association("sBroken",                    QObject::tr("Unused. A lock level string shown in Fallout 3's lockpicking menu, when interacting with a lock that was broken after the player tried and failed to force it.", disambig)),
      _association("sButtonLocked",              QObject::tr("Unused. HUD text displayed when aiming at a door or container with a broken lock. Locks could be broken in Fallout, if an attempt at forcing them failed.", disambig)),
      _association("sCannotCastShout",           QObject::tr("Notification shown when the player attempts to cast a shout while their shouts are on cooldown.", disambig)),
      _association("sCantEquipPowerArmor",       QObject::tr("Unused. Error message displayed when trying to equip Power Armor in Fallout without the appropriate perk.", disambig)),
      _association("sCharGenControlsDisabled",   QObject::tr("Notification shown when the player attempts to perform gameplay actions while gameplay controls are disabled and CharGen has been flagged as active.", disambig)),
      _association("sChemsAddicted",             QObject::tr("Unused. Notification shown in Fallout when a player becomes addicted to a chem.", disambig)),
      _association("sChemsWithdrawal",           QObject::tr("Unused. Notification shown in Fallout when a chem-addicted player is in withdrawal.", disambig)),
      _association("sChemsWornOff",              QObject::tr("Unused. Notification shown in Fallout when a chem that the player has taken wears off.", disambig)),
      _association("sConfirmDisenchant",         QObject::tr("The text of a confirmation prompt shown when the player attempts to disenchant an item using an enchanting station.", disambig)),
      _association("sCursorFilename",            QObject::tr("Unused. The path to the mouse cursor texture; a leftover from Oblivion's UI engine.", disambig)),
      _association("sEffectAlreadyAdded",        QObject::tr("Unused. A leftover from Oblivion's spellmaking menu.", disambig)),
      _association("sLegendaryResetConfirm",       QObject::tr("The text of the first of two confirmation prompts shown when the user attempts to reset a skill, making it Legendary.", disambig)),
      _association("sLegendaryResetSecondConfirm", QObject::tr("The text of the last of two confirmation prompts shown when the user attempts to reset a skill, making it Legendary.", disambig)),
      _association("sMagicGuideNoMarker",        QObject::tr("Notification shown when the player attempts to use a Guide-archetype spell without an active quest set.", disambig)),
      _association("sMagicGuideNoPath",          QObject::tr("Notification shown when the player attempts to use a Guide-archetype spell when the target of their active quest is unreachable.", disambig)),
      _association("sOpenWithKey",               QObject::tr("Notification shown when the player unlocks a door using a key. The format string parameter is the name of the key.", disambig)),
      _association("sOverEncumbered",            QObject::tr("Notification shown when the player is overencumbered. This notification is displayed every thirty seconds of gameplay, or forcibly displayed (resetting the timer) when the player's inventory changes. ", disambig)),
      _association("sPipboyColor",               QObject::tr("Unused. The name of a display option in Fallout, which allows the player to change the color of the Pip-Boy and associated UI.", disambig)),
      _association("sPlayerLeavingBorderRegion", QObject::tr("Notification shown when the player's movement is stopped by the boundary of a border region.", disambig)),
      _association("sRadioSignalLost",           QObject::tr("Unused. Notification shown when a Fallout player exits the broadcast range of a radio station. The format string parameter is the name of the radio station.", disambig)),
      _association("sRadioStationDiscovered",    QObject::tr("Unused. Notification shown when a Fallout player enters the broadcast range of a radio station. The format string parameter is the name of the radio station.", disambig)),
      _association("sSneakCaution",              QObject::tr("Unused. HUD text from Fallout, displayed when the player is sneaking and enemies are searching for them.", disambig)),
      _association("sSneakDanger",               QObject::tr("Unused. HUD text from Fallout, displayed when the player is sneaking and enemies are actively attacking them.", disambig)),
      _association("sSneakDetected",             QObject::tr("Unused. HUD text from Fallout, displayed when the player is sneaking and is detected by non-hostile actors.", disambig)),
      _association("sSneakHidden",               QObject::tr("Unused. HUD text from Fallout, displayed when the player is sneaking and is undetected.", disambig)),
      _association("sSoulCaptured",              QObject::tr("Notification shown when an enemy that the player has previously soul-trapped dies, and one of the player's soul gems is filled.", disambig)),
      _association("sSoulGemTooSmall",           QObject::tr("Notification shown when an enemy that the player has previously soul-trapped dies, but the player doesn't have a soul gem large enough to hold their soul.", disambig)),
      _association("sSpeechChallengeFailure",    QObject::tr("Unused. Text from Fallout's dialogue menu, displayed when the player fails a speech check.", disambig)),
      _association("sSpeechChallengeSuccess",    QObject::tr("Unused. Text from Fallout's dialogue menu, displayed when the player passes a speech check.", disambig)),
      _association("sTweenDisabledMessage",      QObject::tr("Notification shown when the player attempts to open the tween menu (from which they can access the inventory, magic, map, and perk menus) when a script has disabled access to it. The default text tells the player that they cannot access these menus while they are transformed into a werewolf.", disambig)),
      _association("sVATSMessageNoAmmo",         QObject::tr("Unused. VATS HUD text from Fallout, indicating that the player does not have enough ammo for their desired attack.", disambig)),
      _association("sVATSMessageZeroChance",     QObject::tr("Unused. VATS HUD text from Fallout, indicating that the player won't be able to hit a targeted enemy.", disambig)),
      _association("sVDSGManual",                QObject::tr("Unused. Decorative text shown in Fallout's loading screens.", disambig)),
      _association("sVDSGPlate",                 QObject::tr("Unused. Decorative text shown in Fallout's loading screens.", disambig)),
      _association("sWhite",                     QObject::tr("Unused. A color option for Fallout's Pip-Boy and associated UI.", disambig)),
   };
}

extern QString get_game_setting_description(const char* name) {
   if (!name)
      return QString();
   auto length = strlen(name);
   if (length < 2)
      return QString();
   for (auto& entry : _associations)
      if (_strnicmp(name, entry.name, length) == 0)
         return entry.description;
   return QString();
}