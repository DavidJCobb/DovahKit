
## Miscellaneous

* When creating an enchantment, the costliest effect *Effect* (with Magic Effect form *BaseEffect*) will have its magnitude and duration initialized accordingly:
  * Initialize *SoulGemRatio* to the currently selected Soul Gem's soul level value, divided by *iSoulLevelValueGrand*. The Soul Gem's soul level value is one of the game settings from *iSoulLevelValuePetty* through *iSoulLevelValueGrand* &mdash; whichever corresponds to the soul size currently in the gem.
  * Initialize the variable *ScaleProp*.
    * If "Power Affects Magnitude," then set *ScaleProp* is *BaseEffect*'s magnitude (or zero if "No Magnitude").
    * Otherwise, if "Power Affects Duration," then *ScaleProp* is *BaseEffect*'s duration (or zero if "No Duration").
    * Otherwise, *ScaleProp* is 1.0.
  * Let *Skill* be the current value of the player-character's Enchanting actor value.
  * Let *BasePower* be the result of running the *base enchantment power calculation* given *ScaleProp* and *Skill*.
  * Further modify *BasePower* as per the "Modify Enchantment Power" Perk Entry Point.
  * Let *FinalMagnitude* be *BaseEffect*'s magnitude.
  * Let *FinalDuration* be *BaseEffect*'s duration.
  * Let *Power* be 1.0.
  * If the to-be-created enchantment effect has *Parameters*:
    * Set *Power* to *Parameters.Magnitude*, and then clamp *Power* to the range [1, *BasePower*].
    * If *Parameters.MaxMagnitude* is equal to *BasePower*, then set *Power* to *BasePower*.
    * Set *Parameters.MaxMagnitude* to *BasePower*.
    * If we are enchanting an Armor, then set *Parameters.Magnitude* to *Power*.
    * If we are enchanting a Weapon, then set *Parameters.Magnitude* and *Power* to whichever value is higher: 1.0, or *SoulGemRatio* \* *BasePower*.
  * If "Power Affects Magnitude," then set *FinalMagnitude* to *Power*.
  * Otherwise, if "Power Affects Duration," then set *FinalDuration* to *Power* rounded to the nearest integer.
  * Set the costliest effect's magnitude and duration to *FinalMagnitude* and *FinalDuration*.

The effect of the *Parameters* is to downscale an enchantment's power, apparently based on the options the player enters when enchanting (i.e. the slider). *Parameters.Magnitude* is the base effect magnitude scaled by your chosen Soul Gem and (if applicable) your spellmaking power, while *Parameters.MaxMagnitude* is the base effect magnitude scaled only by your spellmaking power (if applicable).

### Base enchantment power calculation

The *base enchantment power calculation* produces the following result, given floats *ScaleProp* and *Skill*:

* Let *esp00* be (*fEnchantingSkillFactor* - 1) \* (*Skill* / 100) + 1.
* Let *esp0C* be (*Skill* - 14) / 85, clamped to a minimum of 0.01.
* Return ((esp00 - 1) * esp0C + 1) * *ScaleProp*.

This formula simplifies to:

* Let *ScaledSkillBoost* be *Skill* / 100 \* (*fEnchantingSkillFactor* - 1).
* Let *FlatSkillBoost* be *Skill* - 14 / 85, clamped to a minimum of 0.01.
* Return *ScaleProp* \* (*ScaledSkillBoost* + *FlatSkillBoost* + 1).

We can draw these conclusions:
* The flat skill boost grants between a 1% and 100% boost to the enchantment's base power, with the percentage increasing linearly for every Enchanting skill point past 15.
* The base power is always at least 1.01 assuming vanilla Game Settings. For this result, you'd have to have an Enchanting skill of 0, and be applying an enchantment whose magnitude and duration don't scale with spellmaking power.