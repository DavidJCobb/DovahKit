
# Actor base

## Specific cases

### Unique actors

Unique actors can only fill quest ref aliases with the "unique actor" fill type if the unique actor has a Persist Location. This is because the game finds unique actors by searching every Location form's "unique actor" ref list.

Reportedly, unique actors are loaded on demand when the player enters their containing Persist Location. However, the player has to enter the location directly, not a child or descendant location. For example, no cells or worldspaces are directly assigned to the "Whiterun Hold" location, so entering cells within Whiterun won't necessarily force-load unique actors that are set to persist in the "Whiterun Hold" location. This means that non-persistent actors that have to move around a hold, or from city to city, have to have their Persist Location set to whatever location their containing worldspace uses. (Apparently, Bethesda forgot to tag the Tamriel worldspace with the "all of Skyrim" location, though, so even that is busted.)

Mods avoid these sorts of problems because the vast majority of mods are ESP files, and all refs in an ESP file are forced to persistent on load.
